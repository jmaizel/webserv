#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/select.h>
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
		int					_port;
		std::string			_host;

	public:
		// Classe canonique
		Server(void);
		Server(int port, const std::string& host);
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
		std::string		ft_handle_request_simple(const std::string& uri);
		std::string		ft_execute_cgi(const std::string& script_path);
		std::string		ft_build_404_response(void);
		std::string		ft_build_403_response(void);
		std::string		ft_build_400_response(void);  // NOUVEAU
		std::string		ft_build_405_response(void);  // NOUVEAU
		
		// Methodes fichiers (ServerFiles.cpp)
		std::string		ft_read_file_simple(const std::string& file_path);
		std::string		ft_get_content_type(const std::string& file_path);
		std::string		ft_serve_static_file(const std::string& uri);
};

#endif