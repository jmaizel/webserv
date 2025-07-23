/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 11:52:04 by jmaizel           #+#    #+#             */
/*   Updated: 2025/07/23 11:52:05 by jmaizel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"

// Constructeur par défaut
Server::Server(void) : _server_fd(-1), _max_fd(0)
{
	// Configuration par défaut
	_config.listen = 8080;
	_config.server_name = "localhost";
	_config.root = "./www";
	_config.index = "index.html";
	
	// Initialiser tous les fd_set à vide
	FD_ZERO(&_master_fds);
	FD_ZERO(&_read_fds);
	FD_ZERO(&_write_fds);
}

// Constructeur avec configuration
Server::Server(const ServerConfig& config) : _server_fd(-1), _max_fd(0), _config(config)
{
	FD_ZERO(&_master_fds);
	FD_ZERO(&_read_fds);
	FD_ZERO(&_write_fds);
}

// Constructeur de copie
Server::Server(const Server& other)
{
	*this = other;  // Utilise l'opérateur d'assignation
}

// Destructeur
Server::~Server(void)
{
	// Fermer le socket serveur s'il existe
	if (_server_fd != -1)
		close(_server_fd);
	
	// Fermer tous les sockets clients
	size_t i = 0;
	while (i < _client_fds.size())
	{
		close(_client_fds[i]);
		i++;
	}
}

// Opérateur d'assignation
Server& Server::operator=(const Server& other)
{
	if (this != &other)  // Protection contre l'auto-assignation
	{
		_server_fd = other._server_fd;
		_address = other._address;
		_max_fd = other._max_fd;
		_client_fds = other._client_fds;
		_config = other._config;  // Copier la config
		_master_fds = other._master_fds;
		_read_fds = other._read_fds;
		_write_fds = other._write_fds;
	}
	return (*this);
}

void Server::ft_init_server(void)
{
	// 1. Créer le socket
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd == -1)
		throw std::runtime_error("Socket creation failed");
	
	// 2. Option SO_REUSEADDR
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Setsockopt failed");
	
	// 3. Rendre le socket non-bloquant
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed");

	// 4. Configurer l'adresse du serveur AVEC LA CONFIG
	_address.sin_family = AF_INET;
	_address.sin_port = htons(_config.listen);  // Utilise _config.listen
	
	// Gérer "localhost" correctement
	if (_config.server_name == "localhost") {
		_address.sin_addr.s_addr = INADDR_ANY;
	} else {
		_address.sin_addr.s_addr = inet_addr(_config.server_name.c_str());
	}

	// 5. Attacher le socket à l'adresse (bind)
	if (bind(_server_fd, (struct sockaddr*)&_address, sizeof(_address)) < 0)
		throw std::runtime_error("Bind failed");
}

void Server::ft_start_listening(void)
{
	// 1. Mettre le socket en mode écoute
	if (listen(_server_fd, MAX_CLIENTS) < 0)
		throw std::runtime_error("Listen failed");
	
	// 2. Ajouter le socket serveur au fd_set principal
	FD_SET(_server_fd, &_master_fds);
	_max_fd = _server_fd;

	std::cout << "Server listening on " << _config.server_name << ":" << _config.listen << std::endl;
}

void Server::ft_handle_connections(void)
{
	while (true)  // Boucle infinie du serveur
	{
		// 1. Copier master_fds dans read_fds (select() modifie read_fds)
		_read_fds = _master_fds;

		// 2. select() attend qu'au moins un socket ait des données
		if (select(_max_fd + 1, &_read_fds, NULL, NULL, NULL) < 0)
			throw std::runtime_error("Select failed");

		// 3. Parcourir TOUS les file descriptors possibles
		int i = 0;
		while (i <= _max_fd)
		{
			// 4. Vérifier si ce fd a des données à lire
			if (FD_ISSET(i, &_read_fds))
			{
				if (i == _server_fd)
				{
					// C'est le socket serveur → nouvelle connexion
					ft_accept_new_client();
				}
				else
				{
					// C'est un client → données à traiter
					ft_handle_client_request(i);
				}
			}
			i++;
		}
	}
}
