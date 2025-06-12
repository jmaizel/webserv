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



void Server:: ft_init_server(void)
{
	// 1. creer le socket
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	// AF_INET = IPv4
	// SOCK_STREAM = TCP (connexion fiable)
	if (_server_fd == -1)
		throw std::runtime_error("Socket creation failed");
	
	// 2. option SO_REUSEADDR
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Fcntl failed");
		// tres important !!! SO_REUSEADDR ca permet de d arreter le serveur et de le relancer directement
		// aussi non ca indique que l adresse est deja utilise
	
	// 3.rendre le socket non bloquant
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK < 0))
		throw std::runtime_error("Fcntl failed");
		// rendre le socket non bloquant , enfait on lance le serveur et on attend indefiniment que le client se connecte
		// en le rendant non bloquant

	// 4.configurer l adresse du serveur
	_address.sin_family = AF_INET;
	_address.sin_port = htons(_port);
	_address.sin_addr.s_addr = inet_addr(_host.c_str());

	// 5.attacher le socket a l adresse (bind)

	if (bind(_server_fd, (struct sockaddr*) &_address, sizeof(_address)) < 0)
		throw std::runtime_error("Bind failed");
}