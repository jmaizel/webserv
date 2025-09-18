/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:42:38 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:42:39 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

std::string get_content_Type(std::string file);

static char fromHex(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    return 0;
}

static std::string urlDecode(const std::string &src)
{
    std::string result;

    for (size_t i = 0; i < src.size(); ++i)
    {
        if (src[i] == '%' && i + 2 < src.size() && std::isxdigit(src[i+1]) && std::isxdigit(src[i+2]))
        {
            char h1 = src[i+1];
            char h2 = src[i+2];
            char decoded = (fromHex(h1) << 4) | fromHex(h2);
            result.push_back(decoded);
            i += 2;
        }
        else if (src[i] == '+')
        {
            result.push_back(' ');
        }
        else
        {
            result.push_back(src[i]);
        }
    }
    return result;
}

std::map<std::string, std::string> parse_raw_body(const std::string &body)
{
    std::map<std::string, std::string> result;
    std::string pair;
    std::istringstream ss(body);

    while (std::getline(ss, pair, '&'))
    {
        size_t pos = pair.find('=');
        if (pos != std::string::npos)
        {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);

            //some chars are transformed by the browser like %10 or space becomes +
            key = urlDecode(key);
            value = urlDecode(value);
            result[key] = value;
        }
    }
    return result;
}

HttpResponse    Server::generate_post_response(HttpRequest &req)
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

    //check if POST is an accepted method in the location
    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "POST") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve POST method");

    //if target is a directory
    if (S_ISDIR(st.st_mode))
    {
        //if the body is a generic query-string like body -> create a file
        std::map<std::string, std::string>::const_iterator it = req.getHeaders().find("Content-Type");
        if (it != req.getHeaders().end() && it->second == "application/x-www-form-urlencoded")
        {
            std::map<std::string, std::string> sorted_body = parse_raw_body(req.getBody());
            it = sorted_body.find("filename");
            if (it == sorted_body.end())
            {
                return generate_error_response(400, "Bad Request", "Missing filename");
            }
            std::string file_path = path + "/" + it->second;
            int fd = open((file_path).c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                std::cout << "HERE : " << file_path << std::endl;
                return generate_error_response(500, " Internel Server Error", "File exists but read failed");
            }
            //if there is content write it
            it = sorted_body.find("content");
            if (it != sorted_body.end())
                write(fd, (it->second).c_str(), (it->second).size());

            close(fd);
        }

        //if we multipart -> relocate a file
        else if (it != req.getHeaders().end() && it->second == "multipart/form-data")
        {
            //this is incorrect. look up how multipart functions
            //int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
            ////open failure
            //if (fd < 0)
            //{
            //    return generate_error_response(500, "Internel Server Error", "File exists but read failed");
            //}
            //write(fd, req.getBody().c_str(), req.getBody().size());
            //close(fd);
        }
        else
        {


        } 
        return generate_success_response(201, "OK", "");
    }

    //if it is a file
    else if (!S_ISREG(st.st_mode))
    {
        return generate_error_response(403, "Forbidden", "Requested ressource is not regular file");
    }

    //unreadable file
    if (access(path.c_str(), R_OK) < 0)
    {
        return generate_error_response(403, "Forbidden", "Requested Ressource is unreadable");
    }
    return (res);

}

