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
    std::string path = "./www" + target;
    std::string parent = path.substr(0, path.find_last_of('/'));

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

    //WATCH OUT HERE WHAT TO DO IF THERE IS NOTHING
    std::string location_path = target.substr(0, target.find_last_of('/'));
    std::cout << "HERE : " << target << " " << location_path << std::endl;
    std::map<std::string, LocationBloc>::iterator it = this->_locations.find(location_path);

    if (it == this->_locations.end())
    {
        generate_error_response(404, "Not Found", "The requested ressource does not exist");
    }

    LocationBloc location = it->second;
    std::cout << parent <<  std::endl;
    location.print();

    //check if DELETE is an accepted method in the location
    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "DELETE") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve DELETE method");
    
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
    if (access(parent.c_str(), W_OK | X_OK) < 0)
    {
        return generate_error_response(403, "Forbidden", "You do not have permissions to access this ressource");
    }

    if (std::remove(path.c_str()))
    {
        return generate_error_response(500, "Internal Server Error", "Unexpected Server Error");
    }

    //Build success response
    return generate_success_response(200, "OK", "Ressource was successfully deleted");
}
