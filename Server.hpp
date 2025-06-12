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

# define MAX_CLIENTS 1024
# define BUFFER_SIZE 4096


class Server
{
    private:
        int _server_fd; // socket du serveur
        struct sockaddr_in _address; // adress du serveur
        fd_set _read_fds; // set pour select() (lecture)
        fd_set _write_fds; // set pour select() (ecriture)
        fd_set _master_fds; // set maitre (tous les fd)
        int _max_fd; // plus grand fd (pour select)
        std::vector<int>	_client_fds;  // liste des clients connectes
        int _port; // port d ecoute
        std::string _host; // IP d ecoute

    public:

    // classe canonique
    Server();
    Server(int port, const std::string& host);
    Server(const Server& other);
    ~Server();

    Server& operator=(const Server& other);

    // methodes
    void ft_init_server(void);
    void ft_start_listening(void);
    void ft_handle_connections(void);
    void ft_accept_new_client(void);
    void ft_handle_client_request(int client_fd);
};
#endif
