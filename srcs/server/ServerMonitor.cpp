/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMonitor.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:39:14 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:39:16 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

int ServerMonitor::_flag = 0;

ServerMonitor::ServerMonitor()
{

}

ServerMonitor::ServerMonitor(std::string &config) : _config(config) , _max_fd(0)
{
    FD_ZERO(&(this->_master_fds));
}


ServerMonitor::~ServerMonitor()
{
    FD_ZERO(&_master_fds);
    for (size_t i = 0; i < _servers.size(); i++)
    {
        _servers[i].shutdown(); // each Server closes its fds and clears them
    }
}

void    ServerMonitor::print()
{
    int j = 1;

    for (size_t i = 0; i < this->_servers.size(); ++i, j++)
    {
       std::cout << "\033[34mSERVER " << j << "\033[0m" << std::endl;
        (this->_servers[i]).print();
        std::cout << "------------------------" << "\n" << std::endl;
    }
    std::cout << std::endl;
}

void ServerMonitor::handle_sigint(int signum)
{
    (void)signum;
    std::cout << "\nSIGINT received, shutting down servers...\n";
    ServerMonitor::_flag = -1;
}

void    ServerMonitor::addServer(Server server)
{
    _servers.push_back(server);
    //add the fd aswell
}

void    ServerMonitor::init_servers()
{
    //init the sockets (FD) of each servers
    for (size_t i = 0; i < this->_servers.size(); i++)
    {
        try { (this->_servers[i]).init();}
        catch (std::exception &e) {throw;}
    }
    
    //add all the servers fd's in master_fds
    size_t i = 0;
    while (i < _servers.size())
    {
        //get the fd from the server
        int server_fd = _servers[i].get_server_fd();

        //add it to the set
        FD_SET(server_fd, &(this->_master_fds));

        //track the biggest fd
        if (server_fd > this->_max_fd)
            this->_max_fd = server_fd;
        i++;
    }
}


void ServerMonitor::run()
{
    if (_servers.empty())
    {
        std::cerr << "No servers to run!" << std::endl;
        return;
    }

    std::cout << "Starting server(s) with " << _servers.size() << " listening socket(s)..." << std::endl;

    fd_set read_fds;

    //main loop01236
    while (ServerMonitor::_flag == 0)
    {
        read_fds = this->_master_fds; //copy master into read_fds

        struct timeval tv;
        tv.tv_sec  = 1;
        tv.tv_usec = 0;

        int activity = select(this->_max_fd + 1, &read_fds, NULL, NULL, &tv);
        //for signal handling
        if (activity < 0)
        {
            if (errno == EINTR)
                break ;
            else if (errno == EBADF)
                throw std::runtime_error("Invalid FD");
            else if (errno == EINVAL)
                throw std::runtime_error("Invalid timeval");
            else
                throw std::runtime_error("Select Error");
        }

        //loop through servers 
        for (size_t i = 0; i < _servers.size(); i++)
        {
            int server_fd = _servers[i].get_server_fd();

            //new connection on this server
            if (FD_ISSET(server_fd, &read_fds))
            {
                _servers[i].accept_new_client();

                int new_client = _servers[i].get_last_client_fd();
                if (new_client > 0)
                {
                    FD_SET(new_client, &(this->_master_fds));
                    if (new_client > this->_max_fd)
                        this->_max_fd = new_client;        
                }
            }

            //loop through all potential client fds
            for (int fd = 0; fd <= this->_max_fd; fd++)
            {

                //check client timeouts
                if (_servers[i].is_client_fd(fd) && fd != server_fd)
                {
                    _servers[i].check_timeouts(TIMEOUT_SEC, fd);
                    if (!_servers[i].is_client_fd(fd))
                    {
                        FD_CLR(fd, &(this->_master_fds));
                    }
                }

                //check activities
                if (FD_ISSET(fd, &read_fds) && fd != server_fd && _servers[i].is_client_fd(fd))
                {
                    _servers[i].handle_client_request(fd);
                    if (!_servers[i].is_client_fd(fd))
                    {
                        FD_CLR(fd, &(this->_master_fds));
                    }
                }
            }
        }
    }
}

bool is_redirect_code(size_t code)
{
    return (code == 301 || code == 302 || code == 303 || code == 307 || code == 308);
}

bool is_error_code(size_t code)
{
    return (code >= 400 && code <= 599);
}

LocationBloc    get_location_bloc(std::vector<std::string> &tokens, std::string &pathi, size_t *i)
{
    LocationBloc                location;
    std::vector<std::string>    key_values;


    location.path = pathi;
    (*i)++;
    for (; tokens[*i] != "}"; ++(*i))
    {
        while (*i < tokens.size() && tokens[*i] != ";" && tokens[*i] != "{" && tokens[*i] != "}")
        {
            key_values.push_back(tokens[*i]);
            (*i)++;
        }
        std::cout << "location key value : ";
        for (size_t j = 0; j < key_values.size(); ++j)
            std::cout << key_values[j] << " ";
        std::cout << std::endl;
        if (key_values.empty())
            continue ;
        if (key_values.size() < 2)
             throw std::runtime_error(key_values[0] + ": no value for this token");

        if (key_values[0] == "root")
        {
            location.root = key_values[1];
        }
        else if (key_values[0] == "index")
        {
            location.index = key_values[1];
        }
        else if (key_values[0] == "allowed_methods")
        {
            location.allowed_methods.clear();
            for (size_t l = 1; l < key_values.size(); ++l)
                location.allowed_methods.push_back(key_values[l]);
        }
        else if (key_values[0] == "client_max_body_size")
        {
            try{location.client_max_body_size = safe_atosize_t(key_values[1]);}
                catch (std::exception &e) {throw std::runtime_error(key_values[0] + ": not a valid number");;}
                location.client_max_body_size_present = true;
        }
        else if (key_values[0] == "autoindex")
        {
            if (key_values[1] == "on")
                location.autoindex = true;
            else if (key_values[1] == "off")
                location.autoindex = false;
            else
            {
                throw std::runtime_error(key_values[1] + ": unkown parameter");
            }
            location.autoindex_bool_present = true;
        }
        else if (key_values[0] == "return")
        {
            //either there are 3 args we have an error code and a raison/redirect 
            //either there are 2 args we only have a error code
            size_t test;
            try{test = safe_atosize_t(key_values[1]);}
                catch (std::exception &e) {throw std::runtime_error(key_values[1] + ": not a valid code");}
            if (!is_error_code(test) && !is_redirect_code(test))
                throw std::runtime_error(key_values[1] + ": not a valid error/redirection code");
            if (!is_redirect_code(test) && key_values.size() < 3)
                throw std::runtime_error(key_values[1] + ": no redirection path after redirection code");
            location.redirect.push_back(key_values[1]);
            if (key_values.size() == 3)
                location.redirect.push_back(key_values[2]);
        }
        else if (key_values[0] == "upload_path")
        {
            location.upload_path = key_values[1];
        }
        else if (key_values[0] == "upload_enable")
        {
            if (key_values[1] == "on")
                location.upload_enable = true;
            else if (key_values[1] == "off")
                location.upload_enable = false;
            else
            {
                throw std::runtime_error(key_values[1] + ": unkown parameter");
            }
            location.upload_bool_present = true;
        }
        else if (key_values[0] == "error_page")
        {
            if (key_values.size() < 3)
                throw std::runtime_error("error_page: not enough parameters");
        
            std::string uri = key_values.back();
            if (uri.empty())
                throw std::runtime_error(uri + ": not a valid URI");
        
            for (size_t i = 1; i < key_values.size() - 1; ++i)
            {
                size_t code;
                try {code = safe_atosize_t(key_values[i]);}
                catch (std::exception &e) { throw std::runtime_error(key_values[i] + ": not a valid number");}
            
                if (!is_error_code(code))
                    throw std::runtime_error(key_values[i] + ": not a valid error code");
            
                location.error_page.push_back(key_values[i]);
            }
            location.error_page.push_back(uri);
        }
        else if (key_values[0] == "cgi_extension")
        {
            if (key_values[1] != ".py")
                throw std::runtime_error(key_values[1] + ": not a valid python extension");
            location.cgi_extension.push_back(key_values[1]);
        }
        //non recognised token
        else 
        {
            throw std::runtime_error(key_values[0] + ": token not recognised");
        }
        key_values.clear();
    }
    return (location);
}

void    validate_locations(ServerBloc &sbloc, std::map<std::string, LocationBloc> &locs)
{
    //first let's check that there is a default location /
    std::map<std::string, LocationBloc>::iterator it;

    //if not create it, it is mandatory so add it
    it = locs.find("/");
    if (it == locs.end())
    {
        std::cout << "no default bloc, creating one..." << std::endl;
        LocationBloc default_loc_bloc(sbloc);
        locs["/"] = default_loc_bloc;
    }
    //whatever is initialized in the default is overwritten by locs
    for (it = locs.begin(); it != locs.end(); ++it)
    {
        //path will nevber be empty
        if (it->second.root.empty())
        {
            it->second.root = sbloc.root;
        }
        if (it->second.allowed_methods.empty())
        {
            it->second.allowed_methods = sbloc.allowed_methods;
        }
        if (it->second.redirect.empty())
        {
            it->second.redirect = sbloc.redirect;
        }
        if (it->second.error_page.empty())
        {
            it->second.error_page = sbloc.error_page;
        }
        if (it->second.cgi_extension.empty())
        {
            it->second.cgi_extension = sbloc.cgi_extension;
        }
        if (it->second.client_max_body_size_present == false)
        {
            it->second.client_max_body_size = sbloc.client_max_body_size;
        }
        if (it->second.index.empty())
        {
            it->second.index = sbloc.index;
        }
        if (it->second.upload_path.empty())
        {
            it->second.upload_path = sbloc.upload_path;
        }
        if (it->second.upload_bool_present == false)
        {
            it->second.upload_enable = sbloc.upload_enable;
        }
        if (it->second.autoindex_bool_present == false)
        {
            it->second.autoindex = sbloc.autoindex;
        }

    }
    sbloc.locations = locs; 
}

ServerBloc  get_server_bloc(std::vector<std::string> &tokens)
{
    std::vector<std::string>    key_values;
    std::map<std::string, LocationBloc> locs;
    ServerBloc                  sbloc;

    for (size_t i = 1; i < tokens.size(); ++i)
    {
        while (i < tokens.size() && tokens[i] != ";" && tokens[i] != "{" && tokens[i] != "}")
        {
            key_values.push_back(tokens[i]);
            i++;
        }
        std::cout << "server key value : ";
        for (size_t j = 0; j < key_values.size(); ++j)
            std::cout << key_values[j] << " ";
        std::cout << std::endl;

        if (key_values.empty())
            continue ;

        if (key_values.size() < 2)
            throw std::runtime_error(key_values[0] + ": no value for this token");

        if (key_values[0] == "allowed_methods")
        {
            sbloc.allowed_methods.clear();
            for (size_t l = 1; l < key_values.size(); ++l)
                sbloc.allowed_methods.push_back(key_values[l]);
        }
        else if (key_values[0] == "listen")
        {
            try{sbloc.listen = safe_atosize_t(key_values[1]);}
            catch (std::exception &e) {throw;}
        }
        else if (key_values[0] == "server_name")
        {
            sbloc.name = key_values[1];
        }
        else if (key_values[0] == "root")
        {
            sbloc.root = key_values[1];
        }
        else if (key_values[0] == "index")
        {
            sbloc.index = key_values[1];
        }
        else if (key_values[0] == "client_max_body_size")
        {
            try{sbloc.client_max_body_size = safe_atosize_t(key_values[1]);}
            catch (std::exception &e) {throw;}
        }
        else if (key_values[0] == "autoindex")
        {
            if (key_values[1] == "on")
                sbloc.autoindex = true;
            else if (key_values[1] == "off")
                sbloc.autoindex = false;
            else
            {
                throw std::runtime_error(key_values[1] + ": unknown parameter");
            }
        }
        else if (key_values[0] == "location")
        {
            try { locs[key_values[1]] = get_location_bloc(tokens, key_values[1], &i);}
            catch (std::exception &e){throw;}
        }
        else if (key_values[0] == "return")
        {
            size_t test;
            if (key_values.size() < 2)
                throw std::runtime_error("return: missing status code");
            if (key_values.size() > 3)
                throw std::runtime_error("return: too may parameters");
            try{test = safe_atosize_t(key_values[1]);}
                catch (std::exception &e) {throw std::runtime_error(key_values[1] + ": not a valid error code");}
            sbloc.redirect.push_back(key_values[1]);
            if (key_values.size() == 3)
                sbloc.redirect.push_back(key_values[2]);
            (void)test;
        }
        else if (key_values[0] == "upload_path")
        {
            sbloc.upload_path = key_values[1];
        }
        else if (key_values[0] == "upload_enable")
        {
            if (key_values[1] == "on")
                sbloc.upload_enable = true;
            else if (key_values[1] == "off")
                sbloc.upload_enable = false;
            else
            {
                throw std::runtime_error(key_values[1] + ": unkown parameter");
            }
        }
        else if (key_values[0] == "error_page")
        {
            if (key_values.size() < 3)
                throw std::runtime_error("error_page: not enough parameters");
        
            std::string uri = key_values.back();
            if (uri.empty())
                throw std::runtime_error(uri + ": not a valid URI");
        
            for (size_t i = 1; i < key_values.size() - 1; ++i)
            {
                size_t code;
                try {code = safe_atosize_t(key_values[i]);}
                catch (std::exception &e) { throw std::runtime_error(key_values[i] + ": not a valid number");}
            
                if (!is_error_code(code))
                    throw std::runtime_error(key_values[i] + ": not a valid error code");
            
                sbloc.error_page.push_back(key_values[i]);
            }
            sbloc.error_page.push_back(uri);
        }
        else if (key_values[0] == "cgi_extension")
        {
            if (key_values[1] != ".py")
                throw std::runtime_error(key_values[1] + ": not a valid python extension");
            sbloc.cgi_extension.push_back(key_values[1]);
        }
        //non recognised token
        else 
        {
            throw std::runtime_error(key_values[0] + ": token not recognised");
        }
        key_values.clear();
    }
    //went through all the tokens. now you need to validate the locations
    validate_locations(sbloc, locs);
    return (sbloc);
}

bool    is_valid_server_bloc(ServerBloc &sbloc)
{
    //a minimal server has: listen, root
    if (sbloc.listen == -1)
        throw (std::runtime_error("Server has no listening port"));
    if (sbloc.root.empty())
        throw (std::runtime_error("Server has no root"));
    if (sbloc.upload_enable && sbloc.upload_path == "")
        throw (std::runtime_error("Server has upload files enabled but no path"));

    std::map<std::string, LocationBloc>::iterator it;
    for (it = sbloc.locations.begin(); it != sbloc.locations.end(); ++it)
    {
        LocationBloc &loc = it->second;

        if (loc.upload_enable && loc.upload_path.empty())
        {
            throw (std::runtime_error("Location " + it->first + " has upload enabled but no path"));
        }
    }

    return true;
}

bool ServerMonitor::valid_semicolons(const std::vector<std::string> &tokens)
{
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string &tok = tokens[i];

        //if it's a block opener
        if (tok == "server")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1] != "{")
                throw std::runtime_error("config file: server must be followed by {");
        }

        else if (tok == "location")
        {
            if (i + 2 >= tokens.size() || tokens[i + 2] != "{")
                throw std::runtime_error("config file: location must be followed by a name and then a {");
            i+=2;
        }
        //if it's a normal directive
        else if (tok != "{" && tok != "}" && tok != ";")
        {
            //find end of this directive
            size_t j = i + 1;
            while (j < tokens.size() && tokens[j] != ";" && tokens[j] != "{" && tokens[j] != "}")
                j++;

            //check we ended with a semicolon
            if (j >= tokens.size() || tokens[j] != ";")
                throw std::runtime_error("config file: missing ; after " + tok);

            //skip ahead to after semicolon
            i = j;
        }
    }
    return true;
}

bool ServerMonitor::valid_brackets(const std::vector<std::string> &tokens)
{
    //checking brackets count
    int depth = 0;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (tokens[i] == "{")
            depth++;
        if (tokens[i] == "}")
            depth--;
        if (depth < 0)
            throw std::runtime_error("config file: too many '}'");
    }

    if (depth != 0)
        throw std::runtime_error("config file: missing closing '}'");

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        //Checks brackets server
        if (tokens[i] == "server")
        {
            if ((i + 1) >= tokens.size() || tokens[i + 1] != "{")
                throw std::runtime_error("config file: missing '{' after server");
            if (i > 0 && tokens[i - 1] != "}")
                throw std::runtime_error("config file: 'server' must start a block or follow '}'");
        }

        //Checks brackets location
        if (tokens[i] == "location")
        {
            if (i + 1 >= tokens.size() || tokens[i + 1] == "{" || tokens[i + 1] == "}" || tokens[i + 1] == ";")
                throw std::runtime_error("config file: missing location path after location");
            if (i + 2 >= tokens.size() || tokens[i + 2] != "{")
                throw std::runtime_error("config file: missing '{' after location");
        }

        //checks stray '{'
        if (tokens[i] == "{")
        {
            if (i == 0)
                throw std::runtime_error("config file: unexpected '{' at start");
            if (tokens[i - 1] != "server" && tokens[i - 2] != "location")
                throw std::runtime_error("config file: unexpected '{' near " + tokens[i-1]);
        }
    }
    return true;
}

std::vector<std::string>    ServerMonitor::tokenize(const std::string &buffer)
{
    std::vector<std::string>    tokens;
    std::string                 current;

    for (size_t i = 0; i < buffer.size(); ++i)
    {
        //end of a token when there is a space
        if (buffer[i] == ' ' || buffer[i] == '}' || buffer[i] == '{' || buffer[i] == ';')
        {
            if (!current.empty())
                tokens.push_back(current);
            if (buffer[i] == '}')
                tokens.push_back("}");
            else if (buffer[i] == '{')
                tokens.push_back("{");
            else if (buffer[i] == ';')
                tokens.push_back(";");
            current.clear();
        }
        else
        {
            current.push_back(buffer[i]);
        }
    }
    if (!current.empty())
        tokens.push_back(current);
    return (tokens);
}

void    ServerMonitor::parse()
{
    std::string                 trimmed;
    std::string                 line;
    std::ostringstream          buffer;
    std::vector<std::string>    tokens;

    //opening file
    std::ifstream   file(_config.c_str());
    if (!file.is_open())
        throw std::runtime_error(_config + ": failed to open");
    //constructing string containing everything
    while (std::getline(file, line))
    {
        trimmed = trimmer(line);
        if (!line.empty())
            buffer << trimmed;
    }
    file.close();
    trimmed = buffer.str();
    std::cout << "BUFFER STRING :\n" << trimmed << "\n" << std::endl; 

    //tokenizing the whole string
    std::cout << "TOKENS : ";
    tokens = tokenize(trimmed);
    for (size_t i = 0; i < tokens.size(); ++i)
            std::cout << tokens[i] << " ";
    std::cout << "\n" << std::endl;
    
    //check bracket placement
    try{valid_brackets(tokens);}
    catch (std::exception &e)    {throw;}

    //check semicolon placement
    try{valid_semicolons(tokens);}
    catch (std::exception &e)    {throw;}

    //tokenize2 : assemble key values, ';' dissapear but brackets remain
    std::vector<std::string>    vserver;
    //looping through all the servers
    for (size_t i = 1; i < tokens.size(); ++i)
    {
        //looping untill a server blov is finished
        while (i < tokens.size() && tokens[i] != "server")
        {
            vserver.push_back(tokens[i]);
            i++;
        }

        std::cout << "SERVER BLOCK" << std::endl;
        for (size_t j = 0; j < vserver.size(); ++j)
            std::cout << vserver[j] << std::endl;
        std::cout << std::endl << std::endl;

        //creating the server bloc
        ServerBloc sbloc = get_server_bloc(vserver);
        vserver.clear();

        //check if it is valid. if so create an actual server
        try{is_valid_server_bloc(sbloc);}
        catch (std::exception &e) {throw;}

            try 
            {
                Server server(sbloc);
                this->addServer(server);
            }
            catch (std::exception &e) {throw;}
    }
}