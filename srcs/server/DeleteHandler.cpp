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

HttpResponse    Server::generate_delete_response(HttpRequest &req, LocationBloc &location)
{
    HttpResponse res;

    std::string target = req.getTarget();

    //check if DELETE is an accepted method in the location
    if(std::find(location.allowed_methods.begin(), location.allowed_methods.end(), "DELETE") == location.allowed_methods.end())
        return generate_error_response(405, "Method Not Allowed", "Requested location doesn't serve DELETE method", location);
    
    //construct the path of the location based on the root
    std::string path = get_ressource_path(target, location);

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
            return generate_error_response(403, "Forbidden", "Directory removal is prohibited", location);
    }

    //unreadable file
    if (access(path.c_str(), F_OK) < 0)
    {
        return generate_error_response(404, "Not Found", "Requested ressource does not exist", location);
    }

    if (std::remove(path.c_str()))
    {
        return generate_error_response(500, "Internal Server Error", "Unexpected Server Error", location);
    }

    //Build success response
    return generate_success_response(200, "OK", "Ressource was successfully deleted");
}
