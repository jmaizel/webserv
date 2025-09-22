/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:41:30 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/22 00:00:00 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

// Déclarations des fonctions CGI (définies dans CgiHandler.cpp)
HttpResponse ft_execute_cgi(const std::string& script_path, const HttpRequest& request, 
                           const std::string& server_name, int server_port);
bool ft_is_cgi_script(const std::string& file_path);

std::string generate_autoindex_string(const std::string &path, const std::string &target)
{
    std::ostringstream stream;

    stream << "<html>\n<body>\n<ul>\n";

    DIR *dir = opendir(path.c_str());
    if (!dir)
    {
        stream << "<li>Permission denied</li>\n";
        stream << "</ul>\n</body>\n</html>\n";
        return stream.str();
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        if (name == "." || name == "..")
            continue;

        if (!name.empty() && name[0] == '.')
            continue;

        struct stat st;
        std::string fullpath = path + "/" + name;
        std::string href = target;
        if (href[href.size() - 1] != '/')
            href += "/";
        href += name;

        if (stat(fullpath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        {
            stream << "<li><a href=\"" << href << "/\">" << name << "/</a></li>\n";
        } 
        else
        {
            stream << "<li><a href=\"" << href << "\">" << name << "</a></li>\n";
        }
    }
    closedir(dir);
    stream << "</ul>\n</body>\n</html>\n";
    return (stream.str());
}

HttpResponse    Server::generate_autoindex_response(const std::string &path, const std::string &target)
{
    std::string body;

    body = generate_autoindex_string(path, target);
    return generate_success_response(200, "OK", body);
}

std::string get_content_Type(std::string file)
{
    size_t      dot_pos;
    std::string content_type;
    std::string ext;

    dot_pos = file.find_last_of('.');
    if (dot_pos != std::string::npos)
        ext = file.substr(dot_pos);
    else 
        ext = "";

    if (ext == ".html" || ext == ".htm")
        content_type = "text/html";
    else if (ext == ".css")
        content_type = "text/css";
    else if (ext == ".js")
        content_type = "application/javascript";
    else if (ext == ".txt")
        content_type = "text/plain";
    else if (ext == ".png")
        content_type = "image/png";
    else if (ext == ".gif")
        content_type = "image/gif";
    else if (ext == ".ico")
        content_type = "image/x-icon";
    else if (ext == ".jpeg" || ext == ".jpg")
        content_type = "image/jpeg";
    else if (ext == ".webp")
        content_type = "image/webp";
    else if (ext == ".svg")
        content_type = "image/svg+xml";
    else if (ext == ".pdf")
        content_type = "application/pdf";
    else if (ext == ".php")
        content_type = "cgi/php";
    else if (ext == ".py")
        content_type = "cgi/py";
    else
        content_type = "application/octet-stream";
    return (content_type);
}
HttpResponse Server::generate_get_response(HttpRequest &req)
{
    HttpResponse res;

    std::string target = req.getTarget();
    std::string path = "./www" + target;

    // DEBUG: Ajouter ces logs
    std::cout << "=== DEBUG GET HANDLER ===" << std::endl;
    std::cout << "Target: '" << target << "'" << std::endl;
    std::cout << "Path: '" << path << "'" << std::endl;
    
    // Vérifier si le fichier www/index.html existe vraiment
    struct stat index_check;
    std::string index_path = "./www/index.html";
    if (stat(index_path.c_str(), &index_check) == 0)
        std::cout << "index.html EXISTS in ./www/" << std::endl;
    else
        std::cout << "index.html NOT FOUND in ./www/ (errno: " << errno << ")" << std::endl;
    
    struct stat st;

    if (stat(path.c_str(), &st) < 0)
    {
        std::cout << "stat() failed for path: " << path << " (errno: " << errno << ")" << std::endl;
        if (errno == EACCES)
            return generate_error_response(403, "Forbidden", "You do not have permission to access this resource.");
        else
        {
            return generate_error_response(404, "Not Found", "The requested resource does not exist.");
        }
    }

    std::cout << "Path exists and stat() succeeded" << std::endl;

    // Detecter et executer les scripts CGI
    if (ft_is_cgi_script(path))
    {
        std::cout << "=== CGI SCRIPT DETECTED ===" << std::endl;
        std::cout << "Executing CGI script: " << path << std::endl;
        
        // Executer le script CGI avec la bonne signature (4 arguments)
        return ft_execute_cgi(path, req, "localhost", 8083);
    }
    else
    {
        std::cout << "NOT a CGI script: " << path << std::endl;
    }

    LocationBloc location = (this->_locations)[target];
    std::cout << "Using location: '" << target << "'" << std::endl;
    std::cout << "Location index: '" << location.index << "'" << std::endl;

    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "GET") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve GET method");
    
    if (S_ISDIR(st.st_mode))
    {
        std::cout << "Path is a directory" << std::endl;
        std::string default_path = path;
        if (default_path[default_path.size() - 1] != '/')
            default_path += "/";

        // CORRECTION ICI : traiter location.index comme un string, pas un vector
        if (!location.index.empty())
        {
            std::string index_file_path = default_path + location.index;
            std::cout << "Checking index file: " << index_file_path << std::endl;
            
            struct stat index_st;
            if (stat(index_file_path.c_str(), &index_st) == 0 && S_ISREG(index_st.st_mode))
            {
                std::cout << "Found index file: " << index_file_path << std::endl;
                path = index_file_path;
                goto serve_file;
            }
            else
            {
                std::cout << "Index file not found or not regular: " << index_file_path << std::endl;
            }
        }
        
        std::cout << "No index file found. Autoindex: " << (location.autoindex ? "ON" : "OFF") << std::endl;
        
        if (location.autoindex == true)
        {
            return generate_autoindex_response(path, target);
        }
        else
        {
            return generate_error_response(403, "Forbidden", "Directory listing is not allowed and no index file was found.");
        }
    }

serve_file:
    std::cout << "Serving file: " << path << std::endl;
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
        return generate_error_response(403, "Forbidden", "You do not have permission to access this resource.");

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::string content_type = get_content_Type(path);

    std::cout << "=== END DEBUG ===" << std::endl;
    return generate_success_response(200, "OK", content);
}