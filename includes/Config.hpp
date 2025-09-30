/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:38:06 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:38:07 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <map>

struct ServerBloc;

struct LocationBloc
{
    std::string                 path;                  
    std::string                 root;                  
    std::vector<std::string>    allowed_methods;
    std::vector<std::string>    redirect;
    std::vector<std::string>    error_page;
    std::vector<std::string>    cgi_extension;
    size_t                      client_max_body_size;       
    std::string                 index;
    std::string                 upload_path;
    bool                        upload_enable;
    bool                        autoindex;
    bool                        upload_bool_present;
    bool                        autoindex_bool_present;
    bool                        client_max_body_size_present;           
    LocationBloc();
    LocationBloc(ServerBloc &s);
    ~LocationBloc();
    void    print();
};

struct ServerBloc
{
    std::string                         root;
    std::string                         name;
    std::string                         index;
    int                                 listen;
    std::vector<std::string>            allowed_methods;
    std::vector<std::string>            redirect;
    std::vector<std::string>            error_page;
    std::vector<std::string>            cgi_extension;
    size_t                              client_max_body_size;
    bool                                autoindex;
    bool                                upload_enable;
    std::map<std::string, LocationBloc> locations;    
    std::string                         upload_path;        
    ServerBloc();
    ~ServerBloc();
    void    print();
};