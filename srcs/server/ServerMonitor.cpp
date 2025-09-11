/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMonitor.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:39:14 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/11 18:00:00 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

ServerMonitor::ServerMonitor()
{

}

ServerMonitor::ServerMonitor(std::string &config) : _config(config)
{

}

ServerMonitor::~ServerMonitor()
{

}

void ServerMonitor::print()
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

void ServerMonitor::addServer(Server server)
{
    _servers.push_back(server);
}

void ServerMonitor::init_servers()
{
    for (size_t i = 0; i < this->_servers.size(); i++)
    {
        try { (this->_servers[i]).init();}
        catch (std::exception &e) {throw;}
    }
}

void ServerMonitor::run()
{
    if (_servers.empty())
    {
        std::cerr << "❌ No servers to run!" << std::endl;
        return;
    }

    std::cout << "🚀 Starting multi-server with " << _servers.size() << " server(s)..." << std::endl;

    // Créer le fd_set principal pour tous les serveurs
    fd_set master_fds, read_fds;
    int max_fd = 0;
    
    FD_ZERO(&master_fds);
    
    // Ajouter tous les serveurs au fd_set
    size_t i = 0;
    while (i < _servers.size())
    {
        int server_fd = _servers[i].get_server_fd();
        FD_SET(server_fd, &master_fds);
        if (server_fd > max_fd)
            max_fd = server_fd;
        i++;
    }

    std::cout << "⚡ Multi-server running... Press Ctrl+C to stop" << std::endl;

    // Boucle principale
    while (true)
    {
        read_fds = master_fds;
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            std::cerr << "❌ Select error" << std::endl;
            break;
        }

        // Vérifier chaque serveur et ses clients
        i = 0;
        while (i < _servers.size())
        {
            int server_fd = _servers[i].get_server_fd();
            
            // Vérifier si ce serveur a une nouvelle connexion
            if (FD_ISSET(server_fd, &read_fds))
            {
                _servers[i].accept_new_client();
                
                // Ajouter le nouveau client au master_fds
                int new_client = _servers[i].get_last_client_fd();
                if (new_client > 0)
                {
                    FD_SET(new_client, &master_fds);
                    if (new_client > max_fd)
                        max_fd = new_client;
                }
            }
            
            // Vérifier tous les file descriptors pour les clients de ce serveur
            int fd = 0;
            while (fd <= max_fd)
            {
                if (FD_ISSET(fd, &read_fds) && fd != server_fd && _servers[i].is_client_fd(fd))
                {
                    _servers[i].handle_client_request(fd);
                    
                    // Si le client s'est déconnecté, le retirer du master_fds
                    if (!_servers[i].is_client_fd(fd))
                    {
                        FD_CLR(fd, &master_fds);
                    }
                }
                fd++;
            }
            
            i++;
        }
    }
}

// ===== TON PARSING ORIGINAL (gardé tel quel) =====

std::string trimmer(std::string &str)
{
    std::string trimmed;

    //all comments supressed
    trimmed = commenttrim(str);
    //trims leading and trailing whitespaces
    trimmed = strtrim(trimmed);
    //allows only one space between elements of a line
    trimmed = spacetrim(trimmed);
    //allows no spaces between brackets
    trimmed = brackettrim(trimmed);
    return (trimmed);
}

std::vector<std::string> tokenize(const std::string &buffer)
{
    std::vector<std::string> tokens;
    std::string current;

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

bool valid_brackets(const std::vector<std::string> &tokens)
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
            throw std::runtime_error("config file: too many closing '}'");
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

bool valid_semicolons(const std::vector<std::string> &tokens)
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

LocationBloc get_location_bloc(std::vector<std::string> &tokens, std::string &pathi, size_t *i)
{
    LocationBloc location;
    std::vector<std::string> key_values;

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
                catch (std::exception &e) {throw;}
        }
        else if (key_values[0] == "autoindex")
        {
            if (key_values[1] == "on")
                location.autoindex = true;
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

ServerBloc get_server_bloc(std::vector<std::string> &tokens)
{
    std::vector<std::string> key_values;
    ServerBloc sbloc;

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
        else if (key_values.size() > 2)
            throw std::runtime_error(key_values[2] + ": unrecognized value");
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
        }
        else if (key_values[0] == "location")
        {
            try { sbloc.locations[key_values[1]] = get_location_bloc(tokens, key_values[1], &i);}
            catch (std::exception &e){throw;}
        }
        //non recognised token
        else 
        {
            throw std::runtime_error(key_values[0] + ": token not recognised");
        }
        key_values.clear();
    }
    return (sbloc);
}

bool is_valid_server_bloc(ServerBloc &sbloc)
{
    //if there are no root or no listen ->error
    if (sbloc.listen == -1)
        return (false);
    if (sbloc.root.empty())
        return (false);
    return (true);
}

void ServerMonitor::parse()
{
    std::string trimmed;
    std::string line;
    std::ostringstream buffer;
    std::vector<std::string> tokens;

    //opening file
    std::ifstream file(_config.c_str());  // Ajouté .c_str() pour C++98
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
    catch (std::exception &e) {throw;}

    //check semicolon placement
    try{valid_semicolons(tokens);}
    catch (std::exception &e) {throw;}

    //tokenize2 : assemble key values, ';' dissapear but brackets remain
    std::vector<std::string> vserver;
    //looping through all the servers
    for (size_t i = 1; i < tokens.size(); ++i)
    {
        //looping untill a server bloc is finished
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
        if (is_valid_server_bloc(sbloc))
        {
            //!this can fail when using sytem calls!
            try 
            {
                Server server(sbloc);
                this->addServer(server);
            }
            catch (std::exception &e) {throw;}
        }
    }
}