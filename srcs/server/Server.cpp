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
      _allowed_methods(1, "GET"),
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
    : _root(s.root),
    _name(s.name),
    _index(s.index),
    _autoindex(false),
    _allowed_methods(s.allowed_methods),
    _client_max_body_size(s.client_max_body_size),
    _locations(s.locations),
    _listen(s.listen),
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
        throw std::runtime_error("bind failed for port " + std::to_string(_listen));
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
    std::string     method = req.getMethod();

    //if request was malformed
    if (req.getFlag() == 400)
    {
        res = generate_invalid_request_response();
    }
    if (method == "GET")
    {
        res = generate_get_response(req);
    }
    else if (method == "POST")
    {
        res = generate_post_response(req);
    }
    else if (method == "DELETE")
    {
        res = generate_delete_response(req);
    }
    else
    {
        res = generate_method_not_implemented_response();
    }
    return (res);
}

void Server::handle_client_request(int client_fd)
{
    //what if i receive a bigger size than 4096????????????????????????????????????????
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