/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HandleErrors.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 14:32:06 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/12 14:32:07 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../includes/main.hpp"

std::string get_reason_phrase(int code)
{
    switch (code)
    {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 303: return "See Other";
        case 307: return "Temporary Redirect";
        case 308: return "Permanent Redirect";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 408: return "Request Timeout";
        case 413: return "Payload Too Large";
        case 415: return "Unsupported Media Type";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        default:  return "Unknown";
    }
}

HttpResponse Server::generate_success_response(int code, const std::string &reason, const std::string &details)
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode(code);
    res.setReason(reason);

    std::string body = "<html><body><h1>" + to_string98(code) + " " + reason + "</h1><p>" + details + "</p></body></html>";
    
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", to_string98(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}

HttpResponse Server::generate_redirect_response(const std::vector<std::string> &redirect, LocationBloc &loc)
{
    //if redirect code is of type 3xx then it is a redirection. use GET to get the ressource
    size_t code = safe_atosize_t(redirect[0]);

    //all the actual redirect codes
    if (code == 301 || code == 302 || code == 303 || code == 307 || code == 308)
    {
        std::cout << redirect[1] << std::endl;
        std::string location = redirect[1];
        HttpResponse res;

        res.setStatusCode(code);
        res.setVersion("HTTP/1.1");
        res.setReason(get_reason_phrase(code));
        res.setHeaders("Location", location);
        res.setHeaders("Content-Type", "text/html");

        std::ostringstream body;
        body << "<html><body><h1>" << code << " " 
             << "Redirected" << "</h1>"
             << "<a href=\"" << location << "\">" << location << "</a>"
             << "</body></html>";

        res.setBody(body.str());
        res.setHeaders("Content-Length", to_string98(body.str().size()));
        return res;
    }

    //otherwise it is just an arbitrary error code with a raison
    if (redirect.size() == 2)
        return generate_error_response(code, redirect[1], "Return option was called", loc);
    return generate_error_response(code, "Unspecified", "Return option was called", loc);
}


HttpResponse Server::generate_get_success_response(int code, const std::string &reason, const std::string &body)
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode(code);
    res.setReason(reason);
    
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", to_string98(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}

HttpResponse Server::generate_custom_error_response(int code, LocationBloc &location)
{
    //checking if we have an error page
    if (location.error_page.empty())
        throw std::runtime_error("empty");

    //isolating uri from error code
    std::string uri = location.error_page.back();
    std::vector<int> error_codes;
    for (size_t i = 0; i < location.error_page.size() - 1; ++i)
    {
        int c = safe_atosize_t(location.error_page[i]);
        error_codes.push_back(c);
    }
    //check if our code matches one of the configured error codes
    for (size_t i = 0; i < error_codes.size(); ++i)
    {
        if (error_codes[i] == code)
        {
            //build the path
            std::string path = location.root + uri;
            std::cout << path << std::endl;

            //check file existence
            if (access(path.c_str(), F_OK) < 0)
            {
                throw std::runtime_error("404");
            }

            //check read permissions
            if (access(path.c_str(), R_OK) < 0)
            {
                throw std::runtime_error("403");
            }

            struct stat st;
            if (stat(path.c_str(), &st) < 0)
                throw std::runtime_error("500");

            if (S_ISDIR(st.st_mode))
                throw std::runtime_error("403");
        
            int fd = open(path.c_str(), O_RDONLY);
            if (fd < 0)
            {
                throw std::runtime_error("500");
            }
        
            //if all succeeded we have a custom error page
            std::string body;
            char buf[4096];
            ssize_t n;
            while ((n = read(fd, buf, sizeof(buf))) > 0)
            {
                body.append(buf, n);
            }
            close(fd);

            //make the http response
            HttpResponse res;
            res.setVersion("HTTP/1.1");
            res.setStatusCode(code);
            res.setReason(get_reason_phrase(code));
            res.setHeaders("Content-Type", "text/html");
            res.setHeaders("Content-Length", to_string98(body.size()));
            res.setHeaders("Connection", "close");
            res.setBody(body);
            res.print();
            return (res);
        }
    }
    throw std::runtime_error("no match");
}

HttpResponse Server::generate_error_response(int code, const std::string &reason, const std::string &details, LocationBloc &location)
{
    HttpResponse res;

    //try to make a custom error page
    try
    {
        res = generate_custom_error_response(code, location);
    }
    catch(const std::exception& e)
    {
        res.setVersion("HTTP/1.1");
        res.setStatusCode(code);
        res.setReason(reason);

        std::string body = "<html><body><h1>" + to_string98(code) + " " + reason + "</h1><p>" + details + "</p></body></html>";

        res.setHeaders("Content-Type", "text/html");
        res.setHeaders("Content-Length", to_string98(body.size()));
        res.setHeaders("Connection", "close");
        res.setBody(body);
    }
    return (res);
}

std::string Server::get_POST_ressource_path(const std::string &target, const LocationBloc &loc)
{
    const std::string &loc_prefix = loc.path;

    std::string relative;
    if (target.size() >= loc_prefix.size())
        relative = target.substr(loc_prefix.size());

    std::string full_path;

    if (loc.upload_enable)
    {
        full_path = loc.upload_path;
        if (!relative.empty())
            full_path += relative;
    }
    else
    {
        full_path = loc.root;
        if (!relative.empty())
            full_path += relative;
    }

    return full_path;
}


std::string Server::get_ressource_path(const std::string &target, const LocationBloc &loc)
{
    const std::string   loc_prefix = loc.path;
    const std::string   root = loc.root;
    std::cout << loc_prefix << " " << root << " " << target << std::endl;

    //hardcoded /

    //strip common part to isolate the relative path
    std::string relative;
    if (target.size() >= loc_prefix.size())
        relative = target.substr(loc_prefix.size());

    //concatenate root and relative path to get full path
    std::string full_path = root;
    if (!relative.empty())
    {
        if (!full_path.empty() && full_path[full_path.size() - 1] != '/' && relative[0] != '/')
            full_path += '/';
        full_path += relative;
    }

    return full_path;
}


std::map<std::string, LocationBloc>::iterator Server::find_best_location(const std::string &target)
{
    std::map<std::string, LocationBloc>::iterator best = this->_locations.end();

    for (std::map<std::string, LocationBloc>::iterator it = this->_locations.begin(); it != this->_locations.end(); ++it)
    {
        const std::string &loc_path = it->first;

        //must match prefix
        if (target.compare(0, loc_path.size(), loc_path) == 0)
        {
            //choose the longest match
            if (best == this->_locations.end() || loc_path.size() > best->first.size())
                best = it;
        }
    }
    return best;
}