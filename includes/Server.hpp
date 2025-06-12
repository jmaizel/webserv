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

		// Methodes principales
		void	ft_init_server(void);
		void	ft_start_listening(void);
		void	ft_handle_connections(void);
		void	ft_accept_new_client(void);
		void	ft_handle_client_request(int client_fd);
		
		// Methodes CGI temporaires
		std::string	ft_execute_cgi(const std::string& script_path);
		std::string	ft_handle_request_simple(const std::string& uri);
};

#endif