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

HttpResponse Server::generate_success_response(int code, const std::string &reason, const std::string &details)
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode(code);
    res.setReason(reason);

    std::string body = "<html><body><h1>" + std::to_string(code) + " " + reason + "</h1><p>" + details + "</p></body></html>";
    
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}

HttpResponse Server::generate_redirect_response(const std::vector<std::string> &redirect)
{
    //if redirect code is of type 3xx then it is a redirection. use GET to get the ressource
    size_t code = safe_atosize_t(redirect[0]);

    //all the actual redirect codes
    if (code == 301 || code == 302 || code == 303 || code == 307 || code == 308)
    {
        std::string location = redirect[1];
        HttpResponse res;

        res.setStatusCode(code);
        res.setReason("Redirected");
        res.setHeaders("Location", location);
        res.setHeaders("Content-Type", "text/html");

        std::ostringstream body;
        body << "<html><body><h1>" << code << " " 
             << "Redirected" << "</h1>"
             << "<a href=\"" << location << "\">" << location << "</a>"
             << "</body></html>";

        res.setBody(body.str());
        res.setHeaders("Content-Length", std::to_string(body.str().size()));
        return res;
    }

    //otherwise it is just an arbitrary error code with a raison
    if (redirect.size() == 2)
        return generate_error_response(code, redirect[1], "Return option was called");
    return generate_error_response(code, "Unspecified", "Return option was called");
}


HttpResponse Server::generate_get_success_response(int code, const std::string &reason, const std::string &body)
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode(code);
    res.setReason(reason);
    
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}

HttpResponse Server::generate_error_response(int code, const std::string &reason, const std::string &details)
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode(code);
    res.setReason(reason);

    std::string body = "<html><body><h1>" + std::to_string(code) + " " + reason + "</h1><p>" + details + "</p></body></html>";

    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}

std::string Server::get_ressource_path(const std::string &target, const LocationBloc &loc)
{
    const std::string &loc_prefix = loc.path;
    const std::string &root = loc.root;

    //strip common part to isolate the relative path
    std::string relative;
    if (target.size() >= loc_prefix.size())
        relative = target.substr(loc_prefix.size());

    //concatenate root and relative path to get full path
    std::string full_path = root;
    if (!relative.empty())
        full_path += relative;

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