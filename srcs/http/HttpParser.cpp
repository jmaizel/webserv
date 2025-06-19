#include "HttpRequest.hpp"
#include <cctype>

// Fonction principale pour parser une requête HTTP complète
HttpRequest ft_parse_http_request(const std::string& raw_data)
{
    HttpRequest request;
    
    // 1. Vérifier que la requête semble complète (contient \r\n\r\n)
    size_t headers_end = raw_data.find("\r\n\r\n");
    if (headers_end == std::string::npos)
    {
        request.error_message = "Incomplete request (missing \\r\\n\\r\\n)";
        return request;
    }
    
    // 2. Séparer la partie headers de la partie body
    std::string headers_part = raw_data.substr(0, headers_end);
    std::string body_part = raw_data.substr(headers_end + 4);  // Skip "\r\n\r\n"
    
    // 3. Extraire la première ligne (request line)
    size_t first_line_end = headers_part.find("\r\n");
    if (first_line_end == std::string::npos)
    {
        request.error_message = "No request line found";
        return request;
    }
    
    std::string request_line = headers_part.substr(0, first_line_end);
    std::string headers_block = headers_part.substr(first_line_end + 2);  // Skip "\r\n"
    
    // 4. Parser la request line
    if (!ft_parse_request_line(request_line, request))
        return request;  // Erreur déjà définie dans la fonction
    
    // 5. Parser les headers
    if (!ft_parse_headers(headers_block, request))
        return request;  // Erreur déjà définie dans la fonction
    
    // 6. Pour POST, récupérer le body si Content-Length existe
    if (request.method == "POST")
    {
        std::map<std::string, std::string>::iterator it = request.headers.find("content-length");
        if (it != request.headers.end())
        {
            int content_length = std::atoi(it->second.c_str());
            if (content_length > 0 && content_length <= static_cast<int>(body_part.length()))
            {
                request.body = body_part.substr(0, content_length);
            }
        }
    }
    
    // 7. Tout est OK !
    request.is_valid = true;
    return request;
}

// Fonction helper pour vérifier si une requête est complète
bool ft_is_request_complete(const std::string& data)
{
    // Vérifier la présence de \r\n\r\n (fin des headers)
    size_t headers_end = data.find("\r\n\r\n");
    if (headers_end == std::string::npos)
        return false;
    
    // Pour GET/DELETE, c'est suffisant
    if (data.find("GET ") == 0 || data.find("DELETE ") == 0)
        return true;
    
    // Pour POST, vérifier qu'on a tout le body selon Content-Length
    if (data.find("POST ") == 0)
    {
        // Chercher Content-Length dans les headers
        size_t cl_pos = data.find("content-length:");
        if (cl_pos == std::string::npos)
            cl_pos = data.find("Content-Length:");
        
        if (cl_pos != std::string::npos && cl_pos < headers_end)
        {
            // Extraire la valeur de Content-Length
            size_t line_end = data.find("\r\n", cl_pos);
            if (line_end != std::string::npos)
            {
                std::string cl_line = data.substr(cl_pos, line_end - cl_pos);
                size_t colon_pos = cl_line.find(':');
                if (colon_pos != std::string::npos)
                {
                    std::string cl_value = cl_line.substr(colon_pos + 1);
                    
                    // Enlever les espaces
                    size_t start = 0;
                    while (start < cl_value.length() && std::isspace(cl_value[start]))
                        start++;
                    cl_value = cl_value.substr(start);
                    
                    int content_length = std::atoi(cl_value.c_str());
                    int body_received = data.length() - (headers_end + 4);
                    
                    return body_received >= content_length;
                }
            }
        }
        return false;  // POST sans Content-Length valide
    }
    
    return true;  // Méthode inconnue, on suppose que c'est complet
}