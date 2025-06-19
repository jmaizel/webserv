#include "Server.hpp"

// Constructeur par défaut
Server::Server(void) : _server_fd(-1), _max_fd(0), _port(8080), _host("127.0.0.1")
{
	// Initialiser tous les fd_set à vide
	FD_ZERO(&_master_fds);
	FD_ZERO(&_read_fds);
	FD_ZERO(&_write_fds);
}

// Constructeur avec paramètres
Server::Server(int port, const std::string& host) : _server_fd(-1), _max_fd(0), _port(port), _host(host)
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
		_port = other._port;
		_host = other._host;
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
	// AF_INET = IPv4, SOCK_STREAM = TCP (connexion fiable)
	if (_server_fd == -1)
		throw std::runtime_error("Socket creation failed");
	
	// 2. Option SO_REUSEADDR (très important !)
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Setsockopt failed");
		// SO_REUSEADDR permet d'arrêter le serveur et de le relancer directement
		// sinon ça indique que l'adresse est déjà utilisée
	
	// 3. Rendre le socket non-bloquant
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed");
		// Rendre le socket non-bloquant permet d'utiliser select()

	// 4. Configurer l'adresse du serveur
	_address.sin_family = AF_INET;
	_address.sin_port = htons(_port);
	
	// Gérer "localhost" correctement
	if (_host == "localhost") {
		_address.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0 (toutes les interfaces)
	} else {
		_address.sin_addr.s_addr = inet_addr(_host.c_str());
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

	std::cout << "Server listening on " << _host << ":" << _port << std::endl;
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