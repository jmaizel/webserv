/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:39:36 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/11 18:00:00 by hsorel           ###   ########.fr       */
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
    _autoindex(s.autoindex),
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
    if (_server_fd != -1)
        close(_server_fd);
    
    // Fermer tous les clients
    size_t i = 0;
    while (i < _client_fds.size())
    {
        close(_client_fds[i]);
        i++;
    }
}

void Server::init()
{
    std::cout << "🔧 Initializing server on port " << _listen << "..." << std::endl;
    
    // Créer le socket
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
        throw std::runtime_error("Socket creation failure");

    // Permettre la réutilisation d'adresse
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        close(_server_fd);
        throw std::runtime_error("Setsockopt failed");
    }

    // Rendre le socket non-bloquant
    if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(_server_fd);
        throw std::runtime_error("Fcntl failed");
    }

    // Configurer l'adresse
    std::memset(&_address, 0, sizeof(_address));
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_listen);

    // Bind le socket à l'adresse
    if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0)
    {
        close(_server_fd);
        std::ostringstream oss;
        oss << "Bind failed on port " << _listen;
        throw std::runtime_error(oss.str());
    }

    // Mettre en écoute
    if (listen(_server_fd, 128) < 0)
    {
        close(_server_fd);
        throw std::runtime_error("Listen failed");
    }

    // Ajouter au fd_set principal
    FD_SET(_server_fd, &_master_fds);
    _max_fd = _server_fd;

    std::cout << "✅ Server initialized on " << _name << ":" << _listen << std::endl;
}

void Server::print()
{
    std::cout << "📋 Server Configuration:" << std::endl;
    std::cout << "   Listen: " << _listen << std::endl;
    std::cout << "   Name: " << _name << std::endl;
    std::cout << "   Root: " << _root << std::endl;
    std::cout << "   Index: " << _index << std::endl;
    std::cout << "   Client max body size: " << _client_max_body_size << std::endl;
    std::cout << "   Autoindex: " << (_autoindex ? "on" : "off") << std::endl;
    
    std::cout << "   Allowed methods: ";
    size_t i = 0;
    while (i < _allowed_methods.size())
    {
        std::cout << _allowed_methods[i];
        if (i < _allowed_methods.size() - 1)
            std::cout << ", ";
        i++;
    }
    std::cout << std::endl;
    
    std::cout << "   Locations (" << _locations.size() << "):" << std::endl;
    std::map<std::string, LocationBloc>::iterator it = _locations.begin();
    while (it != _locations.end())
    {
        std::cout << "     • " << it->first << std::endl;
        std::cout << "       Root: " << (it->second.root.empty() ? "(default)" : it->second.root) << std::endl;
        std::cout << "       Index: " << (it->second.index.empty() ? "(default)" : it->second.index) << std::endl;
        std::cout << "       Autoindex: " << (it->second.autoindex ? "on" : "off") << std::endl;
        std::cout << "       Max body size: " << it->second.client_max_body_size << " bytes" << std::endl;
        ++it;
    }
}

// Ajouter ces méthodes à la fin de ton Server.cpp existant

int Server::get_server_fd(void) const
{
    return _server_fd;
}

int Server::get_last_client_fd(void) const
{
    if (_client_fds.empty())
        return -1;
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
        return; // Pas de client à accepter

    // Rendre le client non-bloquant
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        close(client_fd);
        return;
    }

    // Ajouter à notre surveillance
    FD_SET(client_fd, &_master_fds);
    _client_fds.push_back(client_fd);
    
    if (client_fd > _max_fd)
        _max_fd = client_fd;

    std::cout << "New client connected to server " << _name << " (fd: " << client_fd << ")" << std::endl;
}

void Server::handle_client_request(int client_fd)
{
    char buffer[4096];
    ssize_t bytes_read = recv(client_fd, buffer, 4095, 0);
    
    if (bytes_read <= 0)
    {
        disconnect_client(client_fd);
        return;
    }

    buffer[bytes_read] = '\0';
    std::cout << "Received " << bytes_read << " bytes from client " << client_fd << std::endl;

    // Pour l'instant, juste une réponse simple
    std::ostringstream oss;
    oss << _listen;
    
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: 88\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += "<html><body><h1>Hello from " + _name + ":" + oss.str() + "</h1></body></html>";

    send(client_fd, response.c_str(), response.length(), 0);
    disconnect_client(client_fd);
}

void Server::disconnect_client(int client_fd)
{
    close(client_fd);
    FD_CLR(client_fd, &_master_fds);
    
    // Retirer de la liste des clients
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
    
    std::cout << "Client " << client_fd << " disconnected from server " << _name << std::endl;
}
