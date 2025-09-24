/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:43:14 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:43:15 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

HttpResponse    Server::generate_delete_response(HttpRequest &req)
{
    HttpResponse res;

    std::string target = req.getTarget();

    std::cout << "Delete method called for target : " << target << std::endl;

    //check if there are no redirects in the server
    if (this->_redirect.size() > 1)
        return generate_redirect_response(this->_redirect);

    //check if there is a location match
    std::map<std::string, LocationBloc>::iterator it = find_best_location(target);
    if (it == this->_locations.end())
        return generate_error_response(404, "Not Found", "Requested location does not exist");
    std::cout << "Location matched to : " << it->first << std::endl;
    LocationBloc location = it->second;
    location.print();

    //check if there are no redirects in the location
    if (location.redirect.size() > 1)
        return generate_redirect_response(location.redirect);

    //check if DELETE is an accepted method in the location
    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "DELETE") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve DELETE method");
    
    //construct the path of the location based on the root
    std::string path = get_ressource_path(target, location);
    std::cout << "Contructed ressource path: " << path << std::endl;

    //Check existence of the ressource path
    struct stat st;
    if (stat(path.c_str(), &st) < 0)
    {
        if (errno == EACCES)
            return generate_error_response(403, "Forbidden", "You do not have permission to access this resource.");
        else
        {
            return generate_error_response(404, "Not Found", "The requested resource does not exist.");
        }
    }

    //if it is a directory
    if (S_ISDIR(st.st_mode))
    {
            return generate_error_response(403, "Forbidden", "Directory removal is prohibited");
    }

    //unreadable file
    if (access(path.c_str(), F_OK) < 0)
    {
        return generate_error_response(404, "Not Found", "Requested ressource does not exist");
    }

    //checks parent permissions
    //if (access(parent.c_str(), W_OK | X_OK) < 0)
    //{
    //    return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource");
    //}

    if (std::remove(path.c_str()))
    {
        return generate_error_response(500, "Internal Server Error", "Unexpected Server Error");
    }

    //Build success response
    return generate_success_response(200, "OK", "Ressource was successfully deleted");
}
