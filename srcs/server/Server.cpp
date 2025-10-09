/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:39:36 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:39:37 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

Server::Server()
    : _listen(-1),
      _root(""),
      _name("localhost"),
      _index("index.html"),
      _autoindex(false),
      _upload_enable(false),
       _upload_path(""),
      _allowed_methods(1, "GET"),
      _redirect(),
      _error_page(),
      _cgi_extension(),
      _client_max_body_size(1000000),
      _locations(),
      _server_fd(-1),
      _max_fd(-1),
      _client_buffers()
{
    std::memset(&_address, 0, sizeof(_address));
    FD_ZERO(&_read_fds);
    FD_ZERO(&_write_fds);
    FD_ZERO(&_master_fds);
    _client_fds.clear();
    _client_buffers.clear();
    _clients_timeout.clear();
}



Server::Server(ServerBloc &s)
    :
    _listen(s.listen), 
    _root(s.root),
    _name(s.name),
    _index(s.index),
    _autoindex(s.autoindex),
    _upload_enable(s.upload_enable),
    _upload_path(s.upload_path),
    _allowed_methods(s.allowed_methods),
     _redirect(s.redirect),
    _error_page(s.error_page),
    _cgi_extension(s.cgi_extension),
    _client_max_body_size(s.client_max_body_size),
    _locations(s.locations),
    _server_fd(-1),
    _max_fd(-1)
{
    std::memset(&_address, 0, sizeof(_address));
    FD_ZERO(&_read_fds);
    FD_ZERO(&_write_fds);
    FD_ZERO(&_master_fds);
    _client_fds.clear();
    _client_buffers.clear();
    _clients_timeout.clear();
}


Server::~Server()
{
    this->shutdown();
}

void Server::shutdown()
{
    //closing all client sockets
    for (size_t i = 0; i < _client_fds.size(); i++)
    {
        if (_client_fds[i] >= 0)
            close(_client_fds[i]);
    }
    _client_fds.clear();
    _client_buffers.clear();

    //closing listening socket
    if (_server_fd >= 0)
    {
        close(_server_fd);
        _server_fd = -1;
    }

    FD_ZERO(&_read_fds);
    FD_ZERO(&_write_fds);
    FD_ZERO(&_master_fds);
}


void Server::init()
{
    //creates a socket
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
        throw std::runtime_error("socket creation failure");

    //allows address reuse
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(_server_fd);
        throw std::runtime_error("setsockopt failed");
    }

    //fill address structure
    std::memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_listen);

    //binds socket to IP address
    if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0)
    {
        close(_server_fd);
        throw std::runtime_error("bind failed for port " + to_string98(_listen));
    }

    //start listening -> port opened
    if (listen(_server_fd, SOMAXCONN) < 0)
    {
        close(_server_fd);
        throw std::runtime_error("listen failed");
    }

    //initialize fd_sets
    FD_ZERO(&_read_fds);
    FD_ZERO(&_write_fds);
    FD_ZERO(&_master_fds);
    FD_SET(_server_fd, &_master_fds);

    //Track max fd
    //_max_fd = _server_fd;

    std::cout << "Server listening on port..." << _listen << std::endl;
}


void    Server::print()
{
    std::cout << "listen: " << this->_listen <<  std::endl;
    std::cout << "name: " << this->_name <<  std::endl;
    std::cout << "index: " << this->_index <<  std::endl;
    std::cout << "allowed methods: ";
    for (size_t i = 0 ; i < this->_allowed_methods.size(); ++i)
        std::cout << this->_allowed_methods[i] << " ";
    std::cout << std::endl;
    std::cout << "max body size: " << this->_client_max_body_size <<  std::endl;
    std::cout << "autoindex: " << this->_autoindex <<  std::endl;
    std::cout << "upload enabled: " << this->_upload_enable <<  std::endl;
    std::cout << "upload path: " << this->_upload_path <<  std::endl;
    if (this->_redirect.size() == 1)
        std::cout << "redirect: " << this->_redirect[0] <<  std::endl;
    if (this->_redirect.size() == 2)
        std::cout << "redirect: " << this->_redirect[0] << " " << this->_redirect[1] <<  std::endl;
    if (this->_cgi_extension.size() == 1)
        std::cout << "CGI: " << this->_cgi_extension[0] <<  std::endl;
    if (this->_error_page.size() > 0)
    {
        std::cout << "error page: ";
        for (size_t i = 0; i < this->_error_page.size(); i++)
            std::cout << this->_error_page[i] << " ";
    }
    std::cout << std::endl;
    std::map<std::string, LocationBloc>::iterator it;
    for (it = this->_locations.begin() ; it != this->_locations.end(); ++it)
    {
        std::cout << "location : " << it->first << ": " << std::endl;
        (it->second).print();
        std::cout << std::endl;
    }
}

//jacob

int Server::get_server_fd(void) const
{
    return _server_fd;
}

int Server::get_last_client_fd(void) const
{
    if (_client_fds.empty())
        return (-1);
    return _client_fds.back();
}

bool Server::is_client_fd(int fd) const
{
    size_t i = 0;
    while (i < _client_fds.size())
    {
        if (_client_fds[i] == fd)
            return true;
        i++;
    }
    return false;
}

void Server::accept_new_client(void)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(_server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0)
        return; //no clients to accept
    //make the client non blocking
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(client_fd);
        return;
    }
    if (client_fd > _max_fd)
        _max_fd = client_fd;

    if (_max_fd >= FD_SETSIZE || _client_fds.size() >= MAX_CLIENTS)
    {
        std::string res =
            "HTTP/1.1 503 Service Unavailable\r\n"
            "Connection: close\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        ssize_t bytes = send(client_fd, res.c_str(), res.size(), 0);
        if (bytes < 0)
        {
            disconnect_client(client_fd);
            return ;
        }
        if (bytes == 0)
        {
            return ;
        }
        close(client_fd);
        std::cout << "Refused client (server full) on " << _name << std::endl;
        return;
    }

    //add to clients in the server
    FD_SET(client_fd, &_master_fds);
    _client_fds.push_back(client_fd);
    _client_buffers[client_fd] = std::string("");
    _clients_timeout[client_fd] = time(NULL);
    
    std::cout << "New client id=" << client_fd << " connected to server " << _name << std::endl;
}

HttpResponse    Server::generate_response(HttpRequest &req)
{
    HttpResponse    res;

    //get the method of the request
    std::string     method = req.getMethod();
    //get the target of the request
    std::string     target = req.getTarget();
    //get the corresponding location of the request (there is always one, the default server root
    std::map<std::string, LocationBloc>::iterator   it = find_best_location(target);
    LocationBloc location = it->second;

    if (req.getFlag() == 400)
    {
        res =  res = generate_error_response(400, "Bad Request", "The browser sent a request that this server could not understand", location);
    }
    //check if there are no redirects in the server -> redirect directly
    if (this->_redirect.size() > 1)
        return generate_redirect_response(this->_redirect, location);

    //check if there are no redirects in the location -> redirect directly
    if (location.redirect.size() > 1)
        return generate_redirect_response(location.redirect, location);
    if (method == "GET")
    {
        res = generate_get_response(req, location);
    }
    else if (method == "POST")
    {
        res = generate_post_response(req, location);
    }
    else if (method == "DELETE")
    {
        res = generate_delete_response(req, location);
    }
    //if unknown method
    else
    {
        res = generate_error_response(501, "Not Implemented", "This method is not supported by the server", location);
    }
    return (res);
}

void Server::reset_timeout(int client_fd)
{
    std::map<int, time_t>::iterator it = _clients_timeout.find(client_fd);
    if (it != _clients_timeout.end())
    {
        it->second = std::time(NULL);
    }
}


void Server::check_timeouts(int timeoutSec, int client)
{
    time_t now = std::time(NULL);

    std::map<int, time_t>::iterator it = _clients_timeout.find(client);
    if (it == _clients_timeout.end())
        return ;
    int client_fd = it->first;
    time_t last = it->second;
    if (now - last > timeoutSec)
    {
        //close + remove fd everywhere
        disconnect_client(client_fd);
    }
}

void    Server::generate_error_response_special(int client_fd, int code, const std::string &raison, const std::string &details)
{
    //we are not yet in a location so either use a default one or a special one

    //get the default location
    HttpResponse res;
    std::map<std::string, LocationBloc>::iterator default_location_it;
    LocationBloc location;

    default_location_it = this->_locations.find("/");
    location = default_location_it->second;
    res = generate_error_response(code, raison, details, location);
    ssize_t bytes = send(client_fd, res.toStr().c_str(), res.toStr().size(), MSG_NOSIGNAL);
    if (bytes < 0)
    {
        disconnect_client(client_fd);
        return ;
    }
    if (bytes == 0)
    {
        return ;

    }
}

std::string decode_chunked(std::string &raw)
{
    size_t pos = 0;
    std::string out;

    while (true)
    {
        //find end of first line
        size_t line_end = raw.find("\r\n", pos);
        if (line_end == std::string::npos)
            throw std::runtime_error("no CRLF at the end of a line");

        //isolate size str
        std::string size_str = raw.substr(pos, line_end - pos);

        //parse hex size
        size_t chunk_size;

        try {chunk_size = safe_hextosize_t(size_str);}
        catch (std::exception &e)
        {throw std::runtime_error("Incorrect size line");}

        //skip \r\n after size
        pos = line_end + 2;

        //if end of chunked body
        if (chunk_size == 0)
        {
            if (raw.compare(pos, 2, "\r\n") != 0)
                throw std::runtime_error("Bad termination");
            pos += 4;
            break;
        }

        //get the corresponding string
        std::string content_str = raw.substr(pos, chunk_size);
        if (content_str.size() != chunk_size)
            throw std::runtime_error("Wrong content string");
        //append data
        out.append(content_str);
        pos += chunk_size;

        //must be CRLF after the content string
        if (raw.compare(pos, 2, "\r\n") != 0)
            throw std::runtime_error("Tokens must be terminated by CRLF");
        //skip those
        pos += 2;
    }
    return (out);
}

void Server::handle_client_request(int client_fd)
{
    char buffer[4096];
    
    //get the client buffer
    std::map<int, std::string>::iterator itc;
    itc = _client_buffers.find(client_fd);
    if (itc == _client_buffers.end())
        return;
    std::string &client_buffer = itc->second;

    //read the client information
    ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);

    //if the client interupted the connection
    if (n == 0)
    {
        //no error code here
        std::cout << "Client: " << itc->first << " interrupted the connection" << itc->second << std::endl;
        disconnect_client(client_fd);
        return;
    }

    else if (n < 0)
    {
            disconnect_client(client_fd);
            return ;
    }

    //append client data
    else
    {
        //does the buffer already have the headers?
        if (client_buffer.find("\r\n\r\n") == std::string::npos)
        {
            client_buffer.append(buffer, n);
            //if we still don't have it send it back to select
            if (client_buffer.find("\r\n\r\n") == std::string::npos)
                return;
            //are the headers valid
            HttpRequest req;
            try { req.parse(itc->second); }
            catch (std::exception &e)
            {
                generate_error_response_special(client_fd, 400, "Bad Request", e.what());
                disconnect_client(client_fd);
                return ;
            }
            reset_timeout(client_fd);
            //right when you get the headers clear the buffer
            n = 0;
        }

        HttpRequest req;
        req.parse(itc->second);

        //do we have all the body (Content-Lenght)
        if (req.getMethod() == "POST")
        {
            //get content-length
            std::map<std::string, std::string> headers = req.getHeaders();
            std::map<std::string, std::string>::iterator ith = headers.find("Content-Length");
            std::string body_buffer;

            //if we have content-length
            if (ith != headers.end())
            {
                size_t content_length = safe_atosize_t(ith->second.c_str());
                size_t header_end = client_buffer.find("\r\n\r\n") + 4;
                body_buffer = client_buffer.substr(header_end);
                if (body_buffer.size() < content_length)
                {
                    body_buffer.append(buffer, n);
                    client_buffer.append(buffer, n);
                    //if we still don't have the buffer
                    if (body_buffer.size() < content_length)
                        return;
                    reset_timeout(client_fd);
                }
            }
            //if we have Transfer-Encoding
            else if ((ith = headers.find("Transfer-Encoding")) != headers.end() && ith->second == "chunked")
            {
                //check first 
                size_t header_end = client_buffer.find("\r\n\r\n") + 4;
                body_buffer = client_buffer.substr(header_end);
                //fill untill you have the terminating string
                if (body_buffer.find("0\r\n\r\n") == std::string::npos)
                {
                    //append to the body buffer and the client buffer
                    body_buffer.append(buffer, n);
                    client_buffer.append(buffer, n);
                    //if we still don't have the buffer
                    if (body_buffer.find("0\r\n") == std::string::npos)
                        return;
                    reset_timeout(client_fd);
                }
            }
            //if the body headers is messed up
            else
            {
                generate_error_response_special(client_fd, 400, "Bad Request", "Unkown POST directive");
                disconnect_client(client_fd);
                return;
            }
        }
    }

    //generate the request based on the client buffer
    HttpRequest req;
    req.parse(client_buffer);

    if (req.getMethod() == "POST")
    {
        std::string body;
        std::map<std::string, std::string> headers = req.getHeaders();
        std::map<std::string, std::string>::iterator ith;
        size_t header_end = client_buffer.find("\r\n\r\n") + 4;
        body = client_buffer.substr(header_end);
        
        //if it is chunked
        if ((ith = headers.find("Transfer-Encoding")) != headers.end() && ith->second == "chunked")
        {
            try {body = decode_chunked(body);}
            catch (std::exception &e)
            {
                generate_error_response_special(client_fd, 400, "Bad Request", e.what());
                disconnect_client(client_fd);
                return ;
            }
        }
        req.setBody(body);
    }
    //generate response
    HttpResponse res = generate_response(req);
    std::string response = res.toStr();

    //decide keep-alive or close
    bool keep_alive = true; // default for HTTP/1.1

    std::map<std::string, std::string> headers = req.getHeaders();
    std::map<std::string, std::string>::iterator ith = headers.find("Connection");
    if (ith != headers.end())
    {
        std::string val = ith->second;
        if (val == "close")
            keep_alive = false;
    }

    //add connection header to response
    if (keep_alive)
        res.setHeaders("Connection", "keep-alive");
    else
        res.setHeaders("Connection", "close");

    //send response to client
    ssize_t bytes = send(client_fd, res.toStr().c_str(), res.toStr().size(), MSG_NOSIGNAL);
    if (bytes < 0)
    {
        disconnect_client(client_fd);
        return ;
    }
    if (bytes == 0)
    {
        return ;

    }
    if (keep_alive)
    {
        //reset the timeout
        reset_timeout(client_fd);
        //reset the buffer
        itc->second = std::string("");
        std::cout << "Client kept alive" << std::endl;
    }
    else
    {
        disconnect_client(client_fd);
    }
}

void Server::disconnect_client(int client_fd)
{
    close(client_fd);
    FD_CLR(client_fd, &_master_fds);
    _client_buffers.erase(client_fd);
    _clients_timeout.erase(client_fd);
    
    //remove from client list
    size_t i = 0;
    while (i < _client_fds.size())
    {
        if (_client_fds[i] == client_fd)
        {
            _client_fds.erase(_client_fds.begin() + i);
            break;
        }
        i++;
    }
    std::cout << "Client: " << client_fd << " disconnected from server " << _name << std::endl;
}