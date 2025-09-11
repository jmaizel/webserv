#include "../../includes/ServerConfig.hpp"

std::string ft_trim_line(const std::string& line)
{
    size_t start = 0;
    size_t end = line.length();
    while (start < line.length() && std::isspace(line[start]))
        start++;
    while (end > start && (std::isspace(line[end - 1]) || line[end - 1] == ';'))
        end--;
    return line.substr(start, end - start);
}

void ft_parse_directive(const std::string& line, ServerConfig& config) 
{
    std::string trimmed = ft_trim_line(line);
    std::istringstream iss(trimmed);
    std::string key;
    iss >> key;
    
    if (key == "listen") {
        iss >> config.listen;
    } else if (key == "server_name") {
        iss >> config.server_name;
    } else if (key == "root") {
        iss >> config.root;
    } else if (key == "index") {
        iss >> config.index;
    } else if (key == "client_max_body_size") {
        iss >> config.client_max_body_size;
    } else if (key == "autoindex") {
        std::string value;
        iss >> value;
        config.autoindex = (value == "on");
    } else if (key == "allowed_methods") {
        std::string method;
        config.allowed_methods.clear();  // Clear pour éviter la duplication
        while (iss >> method) {
            config.allowed_methods.push_back(method);
        }
    }
}

void ft_parse_location(std::ifstream& file, ServerConfig& config, const std::string& location_line) 
{
    LocationConfig loc;
    
    // Parser la ligne location pour extraire le path
    std::istringstream iss(location_line);
    std::string location_keyword, path;
    iss >> location_keyword >> path;
    loc.path = path;
    
    // Initialiser avec des valeurs par défaut
    loc.client_max_body_size = 1000000;  // 1MB par défaut
    loc.autoindex = false;
    
    std::string line;
    // Lire les directives du bloc location
    while (std::getline(file, line)) {
        line = ft_trim_line(line);
        if (line.empty()) continue;
        if (line == "}") break; // Fin du bloc
        
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        
        if (key == "root") {
            iss >> loc.root;
        } else if (key == "index") {
            iss >> loc.index;
        } else if (key == "client_max_body_size") {
            iss >> loc.client_max_body_size;
        } else if (key == "autoindex") {
            std::string value;
            iss >> value;
            loc.autoindex = (value == "on");
        } else if (key == "allowed_methods") {
            std::string method;
            loc.allowed_methods.clear();
            while (iss >> method) {
                loc.allowed_methods.push_back(method);
            }
        }
    }
    
    config.locations.push_back(loc);
}

ServerConfig ft_parse_single_server_block(std::ifstream& file)
{
    ServerConfig config;
    
    // Valeurs par défaut
    config.listen = 8080;
    config.server_name = "localhost";
    config.root = "./www";
    config.index = "index.html";
    config.client_max_body_size = 1000000;
    config.autoindex = false;

    std::string line;
    // Lire les directives du bloc server
    while (std::getline(file, line)) {
        line = ft_trim_line(line);
        
        if (line.empty() || line[0] == '#') continue;
        
        if (line == "}") {
            // Fin du bloc server
            break;
        }
        
        if (line.find("location") == 0) {
            ft_parse_location(file, config, line);
        } else {
            ft_parse_directive(line, config);
        }
    }
    
    return config;
}

WebServConfig ft_parse_multiple_servers(const std::string& filename)
{
    WebServConfig webserv_config;
    std::ifstream file(filename.c_str());
    
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        line = ft_trim_line(line);
        
        if (line.empty() || line[0] == '#') continue;

        // Détecter le début d'un bloc server
        if (line.find("server") == 0 && line.find("{") != std::string::npos) {
            // Parser ce bloc server
            ServerConfig server_config = ft_parse_single_server_block(file);
            webserv_config.servers.push_back(server_config);
        }
    }
    
    file.close();
    return webserv_config;
}

// Fonction de compatibilité pour l'ancien parseConfigFile (retourne le premier serveur)
ServerConfig parseConfigFile(const std::string& filename)
{
    WebServConfig webserv_config = ft_parse_multiple_servers(filename);
    
    if (webserv_config.servers.empty()) {
        throw std::runtime_error("No server configuration found in file: " + filename);
    }
    
    return webserv_config.servers[0];  // Retourne le premier serveur pour compatibilité
}