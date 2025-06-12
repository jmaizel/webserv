#include "../includes/Server.hpp"

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

/*
a quoi sert ft_start_listening : 

avant d utiliser cettte fonction on a creer une socket avec socket , mais elle n ecoute pas encore les connexions.
donc apres listen() , la socket devient un "socket serveur" donc il peut recevoir des demandes de connexions, avec MAX_CLIENTS qui est la taille de la fille d attente 

*/
void Server::ft_start_listening(void)
{
	// 1. mettre le socketen mode ecoute
	if (listen(_server_fd, MAX_CLIENTS) < 0)
		throw std::runtime_error("Listen failed");
	
	// 2. ajouter le socket serveur au fd_set principal
	FD_SET(_server_fd, &_master_fds);
	_max_fd = _server_fd;

	std::cout << "Server listening on " << _host << ":" << _port << std::endl;
}


void Server::ft_handle_connections(void)
{
	while(true) //boucle infinie du serveur
	{
		// 1 . copier master_fds dans le read_fds 
		_read_fds = _master_fds;

		//2 select() attend qu au moins un socket ait des donnes = =
		if (select(_max_fd + 1, &_read_fds, NULL, NULL, NULL) < 0)
			throw std::runtime_error ("Select failed");
		// 3. parcourir TOUS les file descriptors possibles
		int i = 0;
		while(i <= _max_fd)
		{
			// 4. verifier si ce fd a des donnes a lire
			if (FD_ISSET(i, &_read_fds))
			{
				if (i == _server_fd)
				{
					// c est le socket serveur -> nouvelle connexion
					ft_accept_new_client();
				}
				else
				{
					// c est un client -> donnees a traiter
					ft_handle_client_request(i);
				}
			}
			i++;
		}

	}
}

void Server::ft_accept_new_client(void)
{
	//1. preparer la structure pour l adresse du client
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// 2. ccepter la connexion
	int client_fd = accept (_server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
	{
		// pas d erreur grave, juste pas de cient maintenant
		return;
	}

	//3. rendre le socket client non-bloquant aussi
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(client_fd);  // Fermer si on peut pas configurer
		return;
	}

	// 4. Ajouter le nouveau client à notre surveillance
	FD_SET(client_fd, &_master_fds);
	_client_fds.push_back(client_fd);
	
	// 5. Mettre à jour le max_fd si nécessaire
	if (client_fd > _max_fd)
		_max_fd = client_fd;

	std::cout << "New client connected: " << client_fd << std::endl;
}

void Server::ft_handle_client_request(int client_fd)
{
	// 1. Buffer pour recevoir les données
	char buffer[BUFFER_SIZE];
	
	// 2. Lire les données du client
	ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	
	// 3. Vérifier si le client s'est déconnecté
	if (bytes_read <= 0)
	{
		// Client déconnecté ou erreur
		std::cout << "Client disconnected: " << client_fd << std::endl;
		
		// Fermer le socket
		close(client_fd);
		
		// Retirer de la surveillance select()
		FD_CLR(client_fd, &_master_fds);
		
		// Retirer de notre liste de clients
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
		return;
	}

	// 4. Terminer la chaîne reçue
	buffer[bytes_read] = '\0';
	
	// 5. Afficher ce qu'on a reçu (debug)
	std::cout << "Received from client " << client_fd << ":" << std::endl;
	std::cout << buffer << std::endl;
	
	// 6. ICI on intégrera le parser HTTP plus tard
	// Pour l'instant, réponse HTTP basique
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: 13\r\n";
	response += "\r\n";
	response += "Hello World!\n";
	
	// 7. Envoyer la réponse au client
	send(client_fd, response.c_str(), response.length(), 0);
}