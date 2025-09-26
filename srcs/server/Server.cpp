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
      _root("./www"),
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
      _max_fd(-1)
{
    std::memset(&_address, 0, sizeof(_address));
    FD_ZERO(&_read_fds);
    FD_ZERO(&_write_fds);
    FD_ZERO(&_master_fds);
    _client_fds.clear();
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

    //add to clients in the server
    FD_SET(client_fd, &_master_fds);
    _client_fds.push_back(client_fd);
    
    if (client_fd > _max_fd)
        _max_fd = client_fd;

    std::cout << "New client connected to server " << _name << std::endl;
}

HttpResponse    Server::generate_response(HttpRequest &req)
{
    HttpResponse    res;

    //get the method of the request
    std::string     method = req.getMethod();
    //get the target of the request
    std::string     target = req.getTarget();
    //get the corresponding location of the request (there is always one, the default location /)
    std::map<std::string, LocationBloc>::iterator   it = find_best_location(target);
    LocationBloc location = it->second;

    //check if there are no redirects in the server -> redirect directly
    if (this->_redirect.size() > 1)
        return generate_redirect_response(this->_redirect, location);

    //check if there are no redirects in the location -> redirect directly
    if (location.redirect.size() > 1)
        return generate_redirect_response(location.redirect, location);

    //if request was malformed
    if (req.getFlag() == 400)
    {
        res =  res = generate_error_response(400, "Bad Request", "The browser sent a request that this server could not understand", location);
    }
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

void Server::handle_client_request(int client_fd)
{
    //ToDo

    //add a timer for a 408! timeout
    //get the headers after a certain amount of time
    //quikly get the content length than i can start comparing -> send 415
    //have a default buffer length 
    //every buffer check if there are some shit inside -> send 400
    //what if i receive a bigger size than 4096????????????????????????????????????????

    //start with getting the headers (set a certain amount of time)
    //then parse the headers if something is wrong -> 400
    //if too big -> 415
    //if took too long -> 408
    
    //THEN get the rest of body
    //set also a timer -> 408
    //if too big -> 415
    //then check for everything and validate it for treatment

    char buffer[4096];
    ssize_t bytes_read = recv(client_fd, buffer, 4095, 0);
    
    //if recv failed or nothing to read
    if (bytes_read <= 0)
    {
        disconnect_client(client_fd);
        return;
    }

    buffer[bytes_read] = '\0';
    std::cout << "Received: " << buffer << std::endl;

    //create a http request object
    HttpRequest req;
    //parse the request based on the buffer
    req.parse(buffer);
    req.print();

    //create a http response object
    HttpResponse res = generate_response(req);
    res.print();
    
    //get the actual string response
    std::string response = res.toStr();

    //send the response to the client fd
    send(client_fd, response.c_str(), response.length(), 0);
    disconnect_client(client_fd);
}

void Server::disconnect_client(int client_fd)
{
    close(client_fd);
    FD_CLR(client_fd, &_master_fds);
    
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
    std::cout << "Client " << " disconnected from server " << _name << std::endl;
}