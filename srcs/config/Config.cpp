/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:39:59 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:40:00 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

LocationBloc::LocationBloc() :
    path("/"),
    root(""),
    allowed_methods(1, "GET"),
    client_max_body_size(1048576),
    index("index.html"),
    autoindex(false)
{

}

LocationBloc::~LocationBloc()
{

}

void    LocationBloc::print()
{
    std::cout << '\t' << path << "\n\t" << root << "\n\t" << client_max_body_size << "\n\t" << index << "\n\t" << autoindex << std::endl;
    for (size_t i = 0; i < allowed_methods.size(); i++)
        std::cout << "\t" << allowed_methods[i] << std::endl;
}

ServerBloc::ServerBloc() : 
    root(""),
    name("localhost"),
    index("index.html"),
    listen(-1),
    allowed_methods(1, "GET"),
    client_max_body_size(1048576),
    autoindex(false),
    locations()
{

}

ServerBloc::~ServerBloc()
{

}

void    ServerBloc::print()
{
    std::cout << listen << "\n" << name << "\n" << root << "\n" << index <<  "\n" << client_max_body_size << "\n" << autoindex << std::endl;;
    for (size_t i = 0; i < allowed_methods.size(); i++)
        std::cout << allowed_methods[i] << std::endl;
    for(std::map<std::string, LocationBloc>::iterator it = locations.begin(); it != locations.end(); ++it)
    {
        std::cout << "location : "<< it->first << std::endl;
        it->second.print();
    }
}
