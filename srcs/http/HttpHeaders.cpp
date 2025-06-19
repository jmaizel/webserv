#include "HttpRequest.hpp"
#include <sstream>
#include <iostream>
#include <cctype>

// Fonction pour trimmer les espaces
std::string ft_trim_header(const std::string& str)
{
    if (str.empty())
        return str;
    
    size_t start = 0;
    size_t end = str.length();
    
    // Trouver le premier caractère non-espace
    while (start < str.length() && std::isspace(str[start]))
        start++;
    
    // Trouver le dernier caractère non-espace
    while (end > start && std::isspace(str[end - 1]))
        end--;
    
    return str.substr(start, end - start);
}

// Convertir en minuscules
std::string ft_to_lower_header(const std::string& str)
{
    std::string result = str;
    size_t i = 0;
    while (i < result.length())
    {
        result[i] = std::tolower(result[i]);
        i++;
    }
    return result;
}

// Parser les headers HTTP
bool ft_parse_headers(const std::string& headers_block, HttpRequest& request)
{
    if (headers_block.empty())
        return true;  // Pas de headers, c'est ok
    
    std::istringstream stream(headers_block);
    std::string line;
    
    while (std::getline(stream, line))
    {
        // Enlever \r en fin de ligne si présent
        if (!line.empty() && line[line.length() - 1] == '\r')
            line.erase(line.length() - 1);
        
        // Ligne vide = fin des headers
        if (line.empty())
            break;
        
        // Trouver le séparateur ':'
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos)
        {
            request.error_message = "Malformed header: " + line;
            return false;
        }
        
        // Extraire key et value
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        // Nettoyer les espaces
        key = ft_trim_header(key);
        value = ft_trim_header(value);
        
        // Vérifier que la clé n'est pas vide
        if (key.empty())
        {
            request.error_message = "Empty header key";
            return false;
        }
        
        // Convertir la clé en minuscules pour uniformité
        key = ft_to_lower_header(key);
        
        // Stocker le header
        request.headers[key] = value;
    }
    
    return true;
}