/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 17:48:36 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 17:48:37 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Config.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdio>
#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>

class Server
{
    private:

        //based on config
        int                                 _listen;
        std::string                         _root;
        std::string                         _name;
        std::string                         _index;
        bool                                _autoindex;
        std::vector<std::string>            _allowed_methods;
        size_t                              _client_max_body_size;
        std::map<std::string, LocationBloc> _locations;

        //basic networking client<->server
        int					_server_fd;
		struct sockaddr_in	_address;
		fd_set				_read_fds;
		fd_set				_write_fds;
		fd_set				_master_fds;
		int					_max_fd;
		std::vector<int>	_client_fds;

    public:
        Server();
        //Server(const Server& other);
        Server(ServerBloc &bloc);
        //Server& operator=(const Server& other);
        ~Server();

        void    init();
        void    print();
};