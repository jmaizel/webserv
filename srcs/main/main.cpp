#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include "ServerConfig.hpp"

int main(int ac, char** av) {
    // Vérifier les arguments
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
        return (1);
    }

    try {
        // Parser la configuration
        ServerConfig config = parseConfigFile(av[1]);

        std::cout << "Server configuration:" << std::endl;
        std::cout << "  Port: " << config.listen << std::endl;
        std::cout << "  Server name: " << config.server_name << std::endl;
        std::cout << "  Root: " << config.root << std::endl;
        std::cout << "  Index: " << config.index << std::endl;
        std::cout << "  Allowed methods: ";
        for (size_t i = 0; i < config.allowed_methods.size(); i++) {
            std::cout << config.allowed_methods[i] << " ";
        }
        std::cout << std::endl;

        std::cout << std::endl << "Locations:" << std::endl;
        for (size_t i = 0; i < config.locations.size(); i++) {
            const LocationConfig& loc = config.locations[i];
            std::cout << "  Location " << i << ":" << std::endl;
            std::cout << "    Path: " << loc.path << std::endl;
            std::cout << "    Root: " << loc.root << std::endl;
            std::cout << "    Index: " << loc.index << std::endl;
            std::cout << "    Allowed methods: ";
            for (size_t j = 0; j < loc.allowed_methods.size(); j++) {
                std::cout << loc.allowed_methods[j] << " ";
            }
            std::cout << std::endl;
            std::cout << "    Client max body size: " << loc.client_max_body_size << std::endl;
            std::cout << std::endl;
        }

        std::cout << "Starting WebServ..." << std::endl;
        
        // Créer le serveur avec la config
        Server server(config.listen, config.server_name);
        
        // Initialiser et démarrer
        server.ft_init_server();
        server.ft_start_listening();
        
        std::cout << "Server ready! Try: http://localhost:" << config.listen << std::endl;
        
        // Boucle principale (parsing HTTP intégré)
        server.ft_handle_connections();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }

    return (0);
}