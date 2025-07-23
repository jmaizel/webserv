#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/select.h>
# include <sys/wait.h> 
# include <unistd.h>
# include <fcntl.h>
# include <iostream>
# include <vector>
# include <map>
# include <string>
# include <stdexcept>
# include <sstream>
# include <cstdio>
# include <fstream>
# include <cstdlib>
# include "ServerConfig.hpp"
# include "HttpRequest.hpp"
# include <sys/stat.h>
# include <cerrno>
# include <cstring>

# define MAX_CLIENTS 1024
# define BUFFER_SIZE 4096

class Server
{
	private:
		int					_server_fd;
		struct sockaddr_in	_address;
		fd_set				_read_fds;
		fd_set				_write_fds;
		fd_set				_master_fds;
		int					_max_fd;
		std::vector<int>	_client_fds;
		ServerConfig		_config;

	public:
		// Classe canonique
		Server(void);
		Server(const ServerConfig& config);
		Server(const Server& other);
		~Server(void);
		Server& operator=(const Server& other);

		// Methodes principales (Server.cpp)
		void			ft_init_server(void);
		void			ft_start_listening(void);
		void			ft_handle_connections(void);
		
		// Methodes réseau (ServerNetwork.cpp)
		void			ft_accept_new_client(void);
		void			ft_handle_client_request(int client_fd);
		void			ft_disconnect_client(int client_fd);
		
		// Methodes HTTP (ServerHttp.cpp)
		std::string 	ft_handle_delete(const std::string& uri);
		std::string		ft_handle_request_simple(const std::string& uri);  // Ancienne méthode
		std::string		ft_handle_request_with_config(const std::string& method, const std::string& uri, const std::string& body);
		
		// CGI - Deux versions pour compatibilité
		std::string		ft_execute_cgi(const std::string& script_path, const HttpRequest& request);
		std::string		ft_build_cgi_response(const std::string& cgi_output);
		std::string		ft_to_upper_env(const std::string& str);
		
		// Réponses d'erreur
		std::string		ft_build_404_response(void);
		std::string		ft_build_403_response(void);
		std::string		ft_build_400_response(void);
		std::string		ft_build_405_response(void);
		std::string		ft_build_413_response(void); 
		
		// Methodes POST (NOUVEAU)
		std::string		ft_handle_post_request_with_config(const std::string& uri, const std::string& body);
		std::map<std::string, std::string>	ft_parse_post_data(const std::string& body);
		std::string		ft_handle_login(const std::map<std::string, std::string>& params);
		std::string		ft_handle_upload(const std::map<std::string, std::string>& params);
		std::string		ft_handle_contact(const std::map<std::string, std::string>& params);
		std::string		ft_get_timestamp(void);
		std::string		ft_build_success_response(const std::string& content, const std::string& content_type);
		std::string		ft_build_401_response(const std::string& content);
		std::string		ft_build_500_response(void);
		std::string		ft_build_post_success_response(const std::string& message);
		
		// Methodes fichiers (ServerFiles.cpp)
		std::string		ft_read_file_simple(const std::string& file_path);
		std::string		ft_get_content_type(const std::string& file_path);
		std::string		ft_serve_static_file(const std::string& uri);  // Ancienne méthode
		std::string		ft_serve_static_file_with_config(const std::string& uri);
		
		// Methodes configuration (ServerConfigMethods.cpp) - NOUVEAU
		const LocationConfig*	ft_find_location(const std::string& uri);
		bool					ft_is_method_allowed(const std::string& method, const LocationConfig* location);
		std::string				ft_get_file_path(const std::string& uri, const LocationConfig* location);
		std::string				ft_build_redirect_response(const std::string& location);
		
		// Helper pour CGI
		std::string		ft_get_directory_path(const std::string& file_path);
};

#endif