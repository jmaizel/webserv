#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

struct LocationConfig {
    std::string path;
    std::string root;
    std::string index;
    std::vector<std::string> allowed_methods;
    int client_max_body_size;
    bool autoindex;
    
    // Constructeur par défaut
    LocationConfig() : client_max_body_size(1000000), autoindex(false) {}
};

struct ServerConfig {
    int listen;
    std::string server_name;
    std::string root;
    std::string index;
    std::vector<std::string> allowed_methods;
    std::vector<LocationConfig> locations;
    std::map<int, std::string> error_pages;
    int client_max_body_size;
    bool autoindex;
    
    // Constructeur par défaut
    ServerConfig() : listen(8080), server_name("localhost"), root("./www"), 
                     index("index.html"), client_max_body_size(1000000), autoindex(false) {}
};

// NOUVELLE STRUCTURE pour gérer plusieurs serveurs
struct WebServConfig {
    std::vector<ServerConfig> servers;
};

// Fonctions de parsing
WebServConfig ft_parse_multiple_servers(const std::string& filename);
ServerConfig ft_parse_single_server_block(std::ifstream& file);

// Fonction de compatibilité (retourne le premier serveur)
ServerConfig parseConfigFile(const std::string& filename);

#endif