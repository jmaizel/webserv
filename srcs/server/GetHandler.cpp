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


HttpResponse    Server::generate_autoindex_response(const std::string &path, const std::string &target)
{
    std::string body;

    body = generate_autoindex_string(path, target);
    return generate_success_response(200, "OK", body);
}

//some files browsers can
std::string get_content_Type(std::string file)
{
    size_t      dot_pos;
    std::string content_type;
    std::string ext;

     //the last dot is the file extension
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
    else if (ext == ".ico")
        content_type = "image/x-icon";
    else
        content_type = "application/octet-stream"; //default (raw bytes)
    return (content_type);
}

HttpResponse Server::generate_get_response(HttpRequest &req)
{
    HttpResponse res;

    std::string target = req.getTarget();
    std::string path = "./www" + target;

    struct stat st;

    //Check existence
    if (stat(path.c_str(), &st) < 0)
    {
        if (errno == EACCES)
            return generate_error_response(403, "Forbidden", "You do not have permission to access this resource.");
        else
        {
            return generate_error_response(404, "Not Found", "The requested resource does not exist.");
        }
    }
    //get the information on that location
    LocationBloc location = (this->_locations)[target];

    //check if GET is an accepted method in the location
    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "GET") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve GET method");
    
    //if it is a directory
    if (S_ISDIR(st.st_mode))
    {
        //look for the default page. if it exists then diplay that
        std::string index_path = path + location.index;
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
            return generate_autoindex_response(path, location.path);
        }
        //if there is no index/wrong permissions and no autoindex -> 403
        else
        {
            return generate_error_response(403, "Forbidden", "Requested location has no index file and no autoindex");
        }
    }

    //if it is a file
    if (!S_ISREG(st.st_mode))
    {
        return generate_error_response(403, "Forbidden", "Requested ressource is not regular file");
    }

    //unreadable file
    if (access(path.c_str(), R_OK) < 0)
    {
        return generate_error_response(403, "Forbidden", "Requested Ressource is unreadable");
    }

    int fd = open(path.c_str(), O_RDONLY);
    //open failure
    if (fd < 0)
    {
        return generate_error_response(500, "Internel Server Error", "File exists bu Read filed"); // unexpected error
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
    res = generate_success_response(200, "OK", body);
    res.setHeaders("Content-Type", get_content_Type(path));
    return (res);
}
