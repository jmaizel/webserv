#include "Server.hpp"
#include <sstream>

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
	if (_server_fd == -1)
		throw std::runtime_error("Socket creation failed");
	
	// 2. Option SO_REUSEADDR (très important !)
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Setsockopt failed");
	
	// 3. Rendre le socket non-bloquant
	if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed");

	// 4. Configurer l'adresse du serveur
	_address.sin_family = AF_INET;
	_address.sin_port = htons(_port);
	//_address.sin_addr.s_addr = inet_addr(_host.c_str());

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

void Server::ft_accept_new_client(void)
{
	// 1. Préparer la structure pour l'adresse du client
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// 2. Accepter la connexion
	int client_fd = accept(_server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
	{
		// Pas d'erreur grave, juste pas de client maintenant
		return;
	}

	// 3. Rendre le socket client non-bloquant aussi
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
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	
	if (bytes_read <= 0)
	{
		std::cout << "Client disconnected: " << client_fd << std::endl;
		close(client_fd);
		FD_CLR(client_fd, &_master_fds);
		
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

	buffer[bytes_read] = '\0';
	std::string raw_data(buffer);
	
	// Parser minimal pour extraire l'URI
	std::string uri = "/";
	size_t first_line_end = raw_data.find("\r\n");
	if (first_line_end != std::string::npos)
	{
		std::string request_line = raw_data.substr(0, first_line_end);
		std::istringstream iss(request_line);
		std::string method, version;
		iss >> method >> uri >> version;
	}
	
	std::cout << "Request: " << uri << std::endl;
	
	// Utiliser notre handler intelligent
	std::string response = ft_handle_request_simple(uri);
	
	send(client_fd, response.c_str(), response.length(), 0);
}

std::string Server::ft_execute_cgi(const std::string& script_path)
{
	// Construire la commande
	std::string command = "python3 " + script_path;
	
	// Exécuter le script et capturer sa sortie
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe)
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 13\r\n\r\nScript failed";
	
	// Lire la sortie ligne par ligne
	std::string result;
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), pipe) != NULL)
	{
		result += buffer;
	}
	pclose(pipe);
	
	// Séparer headers et body du script
	size_t body_start = result.find("\r\n\r\n");
	if (body_start == std::string::npos)
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 17\r\n\r\nMalformed script";
	
	std::string script_headers = result.substr(0, body_start + 2); // Garde \r\n
	std::string body = result.substr(body_start + 4); // Skip \r\n\r\n
	
	// Calculer Content-Length du body
	std::ostringstream oss;
	oss << body.length();
	
	// Construire la réponse complète
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += script_headers; // Headers du script (Content-Type, etc.)
	response += "Content-Length: " + oss.str() + "\r\n";
	response += "\r\n";
	response += body;
	
	return response;
}

std::string Server::ft_handle_request_simple(const std::string& uri)
{
	// 1. Si c'est un script Python → CGI (garde ça)
	if (uri.length() > 3 && uri.substr(uri.length() - 3) == ".py")
	{
		std::string script_path = "./www" + uri;
		return ft_execute_cgi(script_path);
	}
	
	// 2. NOUVEAU : Lire vraiment le fichier
	std::cout << "Trying to serve file for URI: " << uri << std::endl;
	
	// Construire le chemin du fichier
	std::string file_path = "./www";
	if (uri == "/") {
		file_path += "/index.html";  // Page par défaut
	} else {
		file_path += uri;
	}
	
	// Lire le fichier
	std::string file_content = ft_read_file_simple(file_path);
	
	if (file_content.empty()) {
		// Fichier introuvable → Erreur 404
		std::string body = "<html><body>";
		body += "<h1>404 Not Found</h1>";
		body += "<p>Le fichier " + uri + " n'existe pas.</p>";
		body += "<p><a href='/'>Retour à l'accueil</a></p>";
		body += "</body></html>";
		
		std::ostringstream oss;
		oss << body.length();
		
		std::string response = "HTTP/1.1 404 Not Found\r\n";
		response += "Content-Type: text/html\r\n";
		response += "Content-Length: " + oss.str() + "\r\n";
		response += "\r\n";
		response += body;
		
		return response;
	}
	
	// Fichier trouvé → Renvoyer le contenu
	std::ostringstream oss;
	oss << file_content.length();
	
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";  // Pour l'instant tout en HTML
	response += "Content-Length: " + oss.str() + "\r\n";
	response += "\r\n";
	response += file_content;
	
	return response;
}

std::string Server::ft_read_file_simple(const std::string& file_path)
{
	std::cout << "Attempting to read file: " << file_path << std::endl;
	
	std::ifstream file(file_path.c_str());
	if (!file.is_open()) {
		std::cout << "Error: Cannot open file " << file_path << std::endl;
		return "";  // Fichier introuvable
	}
	
	// Lire tout le fichier
	std::string content;
	std::string line;
	
	while (std::getline(file, line)) {
		content += line + "\n";
	}
	
	file.close();
	std::cout << "Successfully read " << content.length() << " bytes from " << file_path << std::endl;
	return content;
}