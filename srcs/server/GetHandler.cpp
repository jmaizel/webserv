/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:41:30 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:41:31 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

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
    //loop trough all the files in a directory
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        //skipping current and previous directory
        if (name == "." || name == "..")
            continue;

        //skipping hidden files
        if (!name.empty() && name[0] == '.')
            continue;

        struct stat st;
        std::string fullpath = path + "/" + name;
        std::string href = target; // not full path! www wil be added in the GET handler
        //add a / to append the name
        if (href[href.size() - 1] != '/')
            href += "/";
        href += name;

        if (stat(fullpath.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
        {
            //if directory
            stream << "<li><a href=\"" << href << "/\">" << name << "/</a></li>\n";
        } else
        {
            //if file
            stream << "<li><a href=\"" << href << "\">" << name << "</a></li>\n";
        }
    }
    closedir(dir);
    stream << "</ul>\n</body>\n</html>\n";
    return (stream.str());
}


HttpResponse    Server::generate_autoindex_response(const std::string &path, const std::string &target, LocationBloc &location)
{
    std::string body;

    if (access(path.c_str(), R_OK | X_OK) < 0)
    {
        return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource", location);
    }
    body = generate_autoindex_string(path, target);
    return generate_get_success_response(200, "OK", body);
}

//some files most browsers can manage
std::string get_content_Type(std::string file)
{
    size_t dot_pos;
    std::string ext;

    dot_pos = file.find_last_of('.');
    if (dot_pos != std::string::npos)
        ext = file.substr(dot_pos);
    else
        ext = "";

    if (ext == ".html" || ext == ".htm")
        return "text/html";
    else if (ext == ".css")
        return "text/css";
    else if (ext == ".js")
        return "application/javascript";
    else if (ext == ".json")
        return "application/json";
    else if (ext == ".xml")
        return "application/xml";
    else if (ext == ".txt")
        return "text/plain";

    //images
    else if (ext == ".png")
        return "image/png";
    else if (ext == ".gif")
        return "image/gif";
    else if (ext == ".ico")
        return "image/x-icon";
    else if (ext == ".jpeg" || ext == ".jpg")
        return "image/jpeg";
    else if (ext == ".webp")
        return "image/webp";
    else if (ext == ".svg")
        return "image/svg+xml";

    //documents
    else if (ext == ".pdf")
        return "application/pdf";

    //audio
    else if (ext == ".mp3")
        return "audio/mpeg";
    else if (ext == ".ogg")
        return "audio/ogg";
    else if (ext == ".wav")
        return "audio/wav";
    else if (ext == ".flac")
        return "audio/flac";

    //video
    else if (ext == ".mp4")
        return "video/mp4";
    else if (ext == ".webm")
        return "video/webm";
    else if (ext == ".ogv")
        return "video/ogg";

    //fonts
    else if (ext == ".woff")
        return "font/woff";
    else if (ext == ".woff2")
        return "font/woff2";
    else if (ext == ".ttf")
        return "font/ttf";
    else if (ext == ".otf")
        return "font/otf";

    //default -> makes raw bytes
    return "application/octet-stream";
}


HttpResponse Server::generate_get_response(HttpRequest &req, LocationBloc &location)
{
    HttpResponse res;

    std::string target = req.getTarget();

    std::cout << "GET method called for target : " << target << std::endl;

    std::cout << "Location matched to : " << location.path << std::endl;
    location.print();

    //check if GET is an accepted method in the location
    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "GET") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve GET method", location);
    
    //construct the path of the location based on the root
    std::string path = get_ressource_path(target, location);
    std::cout << "Contructed ressource path: " << path << std::endl;

    //Check existence of the ressource path
    struct stat st;
    if (stat(path.c_str(), &st) < 0)
    {
        if (errno == EACCES)
            return generate_error_response(403, "Forbidden", "You do not have permission to access this resource.", location);
        else
        {
            return generate_error_response(404, "Not Found", "The requested resource does not exist.", location);
        }
    }

    //if it is a directory
    if (S_ISDIR(st.st_mode))
    {
        //look for the default page. if it exists then diplay that
        std::string index_path = path + "/" + location.index;
        struct stat st_index;

        //checks existence of the index file
        if (stat(index_path.c_str(), &st_index) == 0 && S_ISREG(st_index.st_mode))
        {
            path = index_path;
            st = st_index;
        }

        //if index doesn't exist and there is autoindex display the autoindex
        else if (location.autoindex == true)
        {
            return generate_autoindex_response(path, location.path, location);
        }
        //if there is no index/wrong permissions and no autoindex -> 403
        else
        {
            return generate_error_response(403, "Forbidden", "Requested location has no index file and no autoindex", location);
        }
    }

    //if it is a file

    //check if it is a regular file
    if (!S_ISREG(st.st_mode))
    {
        return generate_error_response(403, "Forbidden", "Requested ressource is not regular file", location);
    }

    //check permissions
    if (access(path.c_str(), R_OK) < 0)
    {
        return generate_error_response(403, "Forbidden", "You do not have to necessary permissions", location);
    }

    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0)
    {
        return generate_error_response(500, "Internel Server Error", "File exists but read failed", location);
    }

    std::string body;
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        body.append(buf, n);
    }
    close(fd);

    //Build success response
    res = generate_get_success_response(200, "OK", body);
    res.setHeaders("Content-Type", get_content_Type(path));
    return (res);
}
