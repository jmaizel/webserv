#ifndef SERVER_MANAGER_HPP
# define SERVER_MANAGER_HPP

# include "Server.hpp"
# include "ServerConfig.hpp"
# include <vector>
# include <iostream>
# include <map>

class ServerManager
{
    private:
        std::vector<Server*> _servers;      // Liste des serveurs
        WebServConfig _config;              // Configuration globale
        fd_set _master_fds;                 // fd_set principal pour tous les serveurs
        fd_set _read_fds;                   // fd_set de lecture
        int _max_fd;                        // Plus grand file descriptor
        
        // Mapper les file descriptors vers les serveurs
        std::map<int, Server*> _fd_to_server;
        
    public:
        ServerManager(void);
        ServerManager(const std::string& config_file);
        ServerManager(const ServerManager& other);
        ~ServerManager(void);
        ServerManager& operator=(const ServerManager& other);
        
        // Méthodes principales
        void ft_init_all_servers(void);
        void ft_start_all_servers(void);
        void ft_run_servers(void);
        void ft_print_config(void);
        
        // Méthodes utilitaires
        void ft_add_server_to_fdset(Server* server);
        Server* ft_find_server_by_fd(int fd);
        void ft_cleanup_servers(void);
};

#endif