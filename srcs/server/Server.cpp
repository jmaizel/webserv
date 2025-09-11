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