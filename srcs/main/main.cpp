#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include "ServerConfig.hpp"

void ft_print_config(const ServerConfig& config)
{
    std::cout << "=== Configuration du serveur ===" << std::endl;
    std::cout << "Port: " << config.listen << std::endl;
    std::cout << "Server name: " << config.server_name << std::endl;
    std::cout << "Root: " << config.root << std::endl;
    std::cout << "Index: " << config.index << std::endl;
    
    std::cout << "Méthodes autorisées: ";
    size_t i = 0;
    while (i < config.allowed_methods.size())
    {
        std::cout << config.allowed_methods[i];
        if (i < config.allowed_methods.size() - 1)
            std::cout << ", ";
        i++;
    }
    std::cout << std::endl;

    std::cout << std::endl << "=== Locations configurées ===" << std::endl;
    i = 0;
    while (i < config.locations.size())
    {
        const LocationConfig& loc = config.locations[i];
        std::cout << "📍 Location [" << i + 1 << "]: " << loc.path << std::endl;
        std::cout << "   Root: " << (loc.root.empty() ? "(défaut serveur)" : loc.root) << std::endl;
        std::cout << "   Index: " << (loc.index.empty() ? "(défaut serveur)" : loc.index) << std::endl;
        std::cout << "   Taille max body: " << loc.client_max_body_size << " bytes" << std::endl;
        
        std::cout << "   Méthodes: ";
        if (loc.allowed_methods.empty())
        {
            std::cout << "(défaut serveur)";
        }
        else
        {
            size_t j = 0;
            while (j < loc.allowed_methods.size())
            {
                std::cout << loc.allowed_methods[j];
                if (j < loc.allowed_methods.size() - 1)
                    std::cout << ", ";
                j++;
            }
        }
        std::cout << std::endl << std::endl;
        i++;
    }
}

int main(int ac, char** av)
{
    // Vérifier les arguments
    if (ac != 2)
    {
        std::cerr << "❌ Usage: " << av[0] << " <config_file>" << std::endl;
        std::cerr << "   Exemple: " << av[0] << " config/config.conf" << std::endl;
        return 1;
    }

    try
    {
        std::cout << "🔧 Chargement de la configuration..." << std::endl;
        
        // Parser la configuration
        ServerConfig config = parseConfigFile(av[1]);
        
        std::cout << "✅ Configuration chargée avec succès !" << std::endl << std::endl;
        
        // Afficher la configuration de manière claire
        ft_print_config(config);
        
        std::cout << "🚀 Démarrage de WebServ..." << std::endl;
        
        // Créer le serveur avec la config
        Server server(config);
        
        // Initialiser et démarrer
        server.ft_init_server();
        server.ft_start_listening();
        
        std::cout << "🌐 Serveur prêt ! Accès: http://" << config.server_name << ":" << config.listen << std::endl;
        std::cout << "📝 Pages disponibles:" << std::endl;
        std::cout << "   • http://localhost:" << config.listen << "/" << std::endl;
        std::cout << "   • http://localhost:" << config.listen << "/login.html" << std::endl;
        std::cout << "   • http://localhost:" << config.listen << "/about.html" << std::endl;
        std::cout << "   • http://localhost:" << config.listen << "/dev.html" << std::endl;
        std::cout << "   • http://localhost:" << config.listen << "/invalidlogin.html" << std::endl;
        std::cout << "   • http://localhost:" << config.listen << "/cgi-bin/time.py" << std::endl;
        std::cout << std::endl << "🔧 Endpoints POST:" << std::endl;
        std::cout << "   • POST /login (authentification)" << std::endl;
        std::cout << "   • POST /upload (upload de fichiers)" << std::endl;
        std::cout << "   • POST /contact (formulaire de contact)" << std::endl;
        std::cout << std::endl << "🔄 En attente de connexions..." << std::endl;
        
        // Boucle principale
        server.ft_handle_connections();
    }
    catch (const std::exception& e)
    {
        std::cerr << "❌ Erreur: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}