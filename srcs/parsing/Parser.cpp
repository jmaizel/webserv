/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/12 11:36:48 by mwattier          #+#    #+#             */
/*   Updated: 2025/06/24 13:32:29 by jmaizel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/ServerConfig.hpp"

// Fonction helper pour trimmer les espaces et enlever les ';'
std::string ft_trim_line(const std::string& line)
{
    size_t start = 0;
    size_t end = line.length();
    
    // Trouver le premier caractère non-espace
    while (start < line.length() && std::isspace(line[start]))
        start++;
    
    // Trouver le dernier caractère non-espace/non-;
    while (end > start && (std::isspace(line[end - 1]) || line[end - 1] == ';'))
        end--;
    
    return line.substr(start, end - start);
}

void parseDirective(const std::string& line, ServerConfig& config) {
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
    } else if (key == "allowed_methods") {
        std::string method;
        while (iss >> method) {
            config.allowed_methods.push_back(method);
        }
    }
}

void parseLocation(std::ifstream& file, ServerConfig& config, const std::string& location_line) {
    LocationConfig loc;
    
    // Parser la ligne location pour extraire le path
    std::istringstream iss(location_line);
    std::string location_keyword, path;
    iss >> location_keyword >> path;
    loc.path = path;
    
    // Initialiser avec des valeurs par défaut
    loc.client_max_body_size = 1000000;  // 1MB par défaut
    
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
        } else if (key == "allowed_methods") {
            std::string method;
            while (iss >> method) {
                loc.allowed_methods.push_back(method);
            }
        } else if (key == "client_max_body_size") {
            iss >> loc.client_max_body_size;
        }
    }
    config.locations.push_back(loc);
}

ServerConfig parseConfigFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    ServerConfig config;
    std::string line;

    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + filename);
    }

    // Valeurs par défaut
    config.listen = 8080;
    config.server_name = "localhost";
    config.root = "./www";
    config.index = "index.html";

    bool in_server_block = false;

    while (std::getline(file, line)) {
        line = ft_trim_line(line);
        
        if (line.empty() || line[0] == '#') continue;

        if (line.find("server") == 0 && line.find("{") != std::string::npos) {
            in_server_block = true;
            continue;
        }
        
        if (line == "}" && in_server_block) {
            in_server_block = false;
            continue;
        }
        
        if (!in_server_block) continue;

        if (line.find("location") == 0) {
            parseLocation(file, config, line);
        } else {
            parseDirective(line, config);
        }
    } 
    file.close();
    return config;
}