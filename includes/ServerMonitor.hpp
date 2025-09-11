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
        std::vector<Server>     _servers;
        fd_set                  _master_fds;
        int                     _max_fd;
        
        //private methods
        std::vector<std::string>    tokenize(const std::string &buffer);
        bool                        valid_brackets(const std::vector<std::string> &tokens);
        bool                        valid_semicolons(const std::vector<std::string> &tokens);
    public:

        //constructors
        ServerMonitor();
        ServerMonitor(std::string &config);

        //destructors
        ~ServerMonitor();

        //public methods
        void    addServer(Server server);
        void    parse();
        void    run();
        void    print();
        void    init_servers();
};

