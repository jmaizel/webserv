#include "Server.hpp"
#include <iostream>
#include <cstdlib>  // Pour atoi()

int main(int argc, char **argv)
{
	try
	{
		int port = 8080;
		std::string host = "127.0.0.1";
		
		// Permettre de changer le port en argument
		if (argc > 1)
			port = atoi(argv[1]);  // Pas std::atoi en C++98
		if (argc > 2)
			host = argv[2];

		std::cout << "Starting WebServ..." << std::endl;
		
		// Créer le serveur
		Server server(port, host);
		
		// Étapes d'initialisation
		server.ft_init_server();
		server.ft_start_listening();
		
		std::cout << "Server ready! Try: http://localhost:" << port << std::endl;
		
		// Boucle principale (bloque ici)
		server.ft_handle_connections();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}

	return (0);
}