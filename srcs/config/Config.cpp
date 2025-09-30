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
    path(""),
    root(""),
    allowed_methods(),
    redirect(),
    error_page(),
    cgi_extension(),
    client_max_body_size(-1),
    index(""),
    upload_path(""),
    upload_enable(false),
    autoindex(false),
    upload_bool_present(false),
    autoindex_bool_present(false),
    client_max_body_size_present(false)
{}

LocationBloc::LocationBloc(ServerBloc &s) :
    path("/"),
    root(s.root),
    allowed_methods(s.allowed_methods),
    redirect(s.redirect),
    error_page(s.error_page),
    cgi_extension(s.cgi_extension),
    client_max_body_size(s.client_max_body_size),
    index(s.index),
    upload_path(s.upload_path),
    upload_enable(false),
    autoindex(false)
{}

LocationBloc::~LocationBloc()
{

}

void    LocationBloc::print()
{
    std::cout << '\t' << path << "\n\t" << root << "\n\t" << client_max_body_size << "\n\t" << index << "\n\t" << autoindex << "\n\t" << upload_path << "\n\t" << upload_enable << std::endl;
    if (redirect.size() > 0)
        std::cout << "\t" << redirect[0];
    if (redirect.size() > 1)
        std::cout << " " << redirect[1];
    if (this->cgi_extension.size() == 1)
        std::cout << "\tCGI: " << this->cgi_extension[0] <<  std::endl;
    if (error_page.size() > 0)
    {
        std::cout << "\terror page: ";
        for (size_t i = 0; i < error_page.size(); i++)
            std::cout << error_page[i] << " ";
    }
    std::cout << std::endl;
    for (size_t i = 0; i < allowed_methods.size(); i++)
        std::cout << "\t" << allowed_methods[i] << std::endl;
}

ServerBloc::ServerBloc() : 
    root(""),
    name("localhost"),
    index("index.html"),
    listen(-1),
    allowed_methods(1, "GET"),
    redirect(),
    error_page(),
    cgi_extension(),
    client_max_body_size(1048576),
    autoindex(false),
    upload_enable(false),
    locations(),
    upload_path("")
{

}

ServerBloc::~ServerBloc()
{

}

void    ServerBloc::print()
{
    std::cout << listen << "\n" << name << "\n" << root << "\n" << index <<  "\n" << client_max_body_size << "\n" << autoindex << "\n\t" << upload_path << "\n\t" << upload_enable << "\n\t"  << redirect[0] << "\n\t" << redirect[1] << std::endl;
    for (size_t i = 0; i < allowed_methods.size(); i++)
        std::cout << allowed_methods[i] << std::endl;
    for(std::map<std::string, LocationBloc>::iterator it = locations.begin(); it != locations.end(); ++it)
    {
        std::cout << "location : "<< it->first << std::endl;
        it->second.print();
    }
}
