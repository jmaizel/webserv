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
#include "Client.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

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
        bool                                _upload_enable;
        std::string                         _upload_path;
        std::vector<std::string>            _allowed_methods;
        std::vector<std::string>            _redirect;
        std::vector<std::string>            _error_page;
        std::vector<std::string>            _cgi_extension;
        size_t                              _client_max_body_size;
        std::map<std::string, LocationBloc> _locations;

        //basic networking client<->server
        int					_server_fd;
		struct sockaddr_in	_address;
		fd_set				_read_fds;
		fd_set				_write_fds;
		fd_set				_master_fds;
		int					_max_fd;
		std::vector<int>            _client_fds;
        std::map<int, std::string>  _client_buffers;
        std::map<int, time_t>       _clients_timeout;

        //helpers
        std::map<std::string, LocationBloc>::iterator   find_best_location(const std::string &target);
        std::string     get_ressource_path(const std::string &target, const LocationBloc &loc);
        std::string     get_POST_ressource_path(const std::string &target, const LocationBloc &loc);
        HttpResponse    handle_file_response(const std::string &target, LocationBloc &location, const std::string &body, int flag);
        HttpResponse    handle_url_encoded(const std::string &body, const std::string &path, LocationBloc &location);
        HttpResponse    handle_json(const std::string &body, const std::string &path, LocationBloc &location);
        HttpResponse    handle_multipart(const std::string &body, const std::string &path, const std::string &content_type, LocationBloc &location);
        std::string     get_boundary(const std::string &content_type);

    public:
        Server();
        //Server(const Server& other);
        Server(ServerBloc &bloc);
        //Server& operator=(const Server& other);
        ~Server();


        //networking methods
		int     get_server_fd(void) const;
		int     get_last_client_fd(void) const;
		bool    is_client_fd(int fd) const;
		void    accept_new_client(void);
		void    disconnect_client(int client_fd);
        void    check_timeouts(int timeoutSec, int client);
        void    reset_timeout(int client_fd);

        void    init();
        void    print();
        void    shutdown();

        //response methods
        void            handle_client_request(int client_fd);
        HttpResponse    generate_response(HttpRequest &req);
        HttpResponse    generate_success_response(int code, const std::string &reason, const std::string &body);
        HttpResponse    generate_get_success_response(int code, const std::string &reason, const std::string &body);
        HttpResponse    generate_autoindex_response(const std::string &path, const std::string &target, LocationBloc &location);
        HttpResponse    generate_get_response(HttpRequest &req, LocationBloc &location);
        HttpResponse    generate_post_response(HttpRequest &req, LocationBloc &location);
        HttpResponse    generate_delete_response(HttpRequest &req, LocationBloc &location);
        HttpResponse    generate_error_response(int code, const std::string &reason, const std::string &details, LocationBloc &location);
        HttpResponse    generate_custom_error_response(int code, LocationBloc &location);
        HttpResponse    generate_redirect_response(const std::vector<std::string> &redirect, LocationBloc &location);
        HttpResponse    generate_cgi_response(const std::string& script_path, const HttpRequest& request, LocationBloc &location);
};