/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMonitor.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 18:00:37 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 18:00:38 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Server.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

class ServerMonitor
{
    private:
        std::string             _config;
        std::vector<Server>    _servers;
        
    public:
        ServerMonitor();
        ServerMonitor(std::string &config);
        ~ServerMonitor();

        //methods
        void    addServer(Server server);
        void    parse();
        void    run();
        void    print();
        void    init_servers();
};

