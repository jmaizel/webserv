#include "../../includes/ServerManager.hpp"

ServerManager::ServerManager(void) : _max_fd(0)
{
    FD_ZERO(&_master_fds);
    FD_ZERO(&_read_fds);
}

ServerManager::ServerManager(const std::string& config_file) : _max_fd(0)
{
    FD_ZERO(&_master_fds);
    FD_ZERO(&_read_fds);
    
    // Parser la configuration
    _config = ft_parse_multiple_servers(config_file);
    
    // Créer un serveur pour chaque configuration
    size_t i = 0;
    while (i < _config.servers.size())
    {
        Server* server = new Server(_config.servers[i]);
        _servers.push_back(server);
        i++;
    }
}

ServerManager::ServerManager(const ServerManager& other)
{
    *this = other;
}

ServerManager::~ServerManager(void)
{
    ft_cleanup_servers();
}

ServerManager& ServerManager::operator=(const ServerManager& other)
{
    if (this != &other)
    {
        ft_cleanup_servers();
        _config = other._config;
        _max_fd = other._max_fd;
        _master_fds = other._master_fds;
        _read_fds = other._read_fds;
        _fd_to_server = other._fd_to_server;
        
        // Créer de nouveaux serveurs
        size_t i = 0;
        while (i < _config.servers.size())
        {
            Server* server = new Server(_config.servers[i]);
            _servers.push_back(server);
            i++;
        }
    }
    return (*this);
}

void ServerManager::ft_cleanup_servers(void)
{
    size_t i = 0;
    while (i < _servers.size())
    {
        delete _servers[i];
        i++;
    }
    _servers.clear();
    _fd_to_server.clear();
}

void ServerManager::ft_init_all_servers(void)
{
    std::cout << "🔧 Initialisation de " << _servers.size() << " serveur(s)..." << std::endl;
    
    size_t i = 0;
    while (i < _servers.size())
    {
        try
        {
            _servers[i]->ft_init_server();
            std::cout << "✅ Serveur " << (i + 1) << " initialisé (port " 
                      << _config.servers[i].listen << ")" << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "❌ Erreur serveur " << (i + 1) << ": " << e.what() << std::endl;
            throw;
        }
        i++;
    }
}

void ServerManager::ft_start_all_servers(void)
{
    std::cout << "🚀 Démarrage de tous les serveurs..." << std::endl;
    
    // Initialiser le fd_set principal
    FD_ZERO(&_master_fds);
    _max_fd = 0;
    
    size_t i = 0;
    while (i < _servers.size())
    {
        try
        {
            _servers[i]->ft_start_listening();
            ft_add_server_to_fdset(_servers[i]);
            
            std::cout << "🌐 Serveur " << (i + 1) << " écoute sur " 
                      << _config.servers[i].server_name << ":" 
                      << _config.servers[i].listen << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "❌ Erreur démarrage serveur " << (i + 1) << ": " << e.what() << std::endl;
            throw;
        }
        i++;
    }
}

void ServerManager::ft_add_server_to_fdset(Server* server)
{
    int server_fd = server->ft_get_server_fd();  // Méthode à ajouter dans Server
    
    FD_SET(server_fd, &_master_fds);
    _fd_to_server[server_fd] = server;
    
    if (server_fd > _max_fd)
        _max_fd = server_fd;
}

Server* ServerManager::ft_find_server_by_fd(int fd)
{
    std::map<int, Server*>::iterator it = _fd_to_server.find(fd);
    if (it != _fd_to_server.end())
        return it->second;
    
    // Si ce n'est pas un fd serveur, chercher dans les clients de chaque serveur
    size_t i = 0;
    while (i < _servers.size())
    {
        if (_servers[i]->ft_is_client_fd(fd))  // Méthode à ajouter dans Server
            return _servers[i];
        i++;
    }
    
    return NULL;
}

void ServerManager::ft_run_servers(void)
{
    std::cout << "⚡ Serveurs en fonctionnement..." << std::endl;
    std::cout << "Appuyez sur Ctrl+C pour arrêter" << std::endl;
    
    while (true)
    {
        // Copier master_fds dans read_fds
        _read_fds = _master_fds;
        
        // Attendre une activité sur n'importe quel socket
        if (select(_max_fd + 1, &_read_fds, NULL, NULL, NULL) < 0)
        {
            std::cerr << "❌ Erreur select()" << std::endl;
            throw std::runtime_error("Select failed");
        }
        
        // Parcourir tous les file descriptors
        int i = 0;
        while (i <= _max_fd)
        {
            if (FD_ISSET(i, &_read_fds))
            {
                Server* server = ft_find_server_by_fd(i);
                if (server)
                {
                    // Vérifier si c'est un socket serveur ou client
                    std::map<int, Server*>::iterator it = _fd_to_server.find(i);
                    if (it != _fd_to_server.end())
                    {
                        // C'est un socket serveur → nouvelle connexion
                        server->ft_accept_new_client();
                        
                        // Ajouter les nouveaux clients au master_fds
                        int new_client_fd = server->ft_get_last_client_fd();  // Méthode à ajouter
                        if (new_client_fd > 0)
                        {
                            FD_SET(new_client_fd, &_master_fds);
                            if (new_client_fd > _max_fd)
                                _max_fd = new_client_fd;
                        }
                    }
                    else
                    {
                        // C'est un client → traiter la requête
                        server->ft_handle_client_request(i);
                        
                        // Vérifier si le client s'est déconnecté
                        if (!server->ft_is_client_fd(i))
                        {
                            FD_CLR(i, &_master_fds);
                        }
                    }
                }
            }
            i++;
        }
    }
}

void ServerManager::ft_print_config(void)
{
    std::cout << "📋 Configuration WebServ:" << std::endl;
    std::cout << "Nombre de serveurs: " << _config.servers.size() << std::endl << std::endl;
    
    size_t i = 0;
    while (i < _config.servers.size())
    {
        const ServerConfig& server = _config.servers[i];
        
        std::cout << "🌐 SERVEUR " << (i + 1) << ":" << std::endl;
        std::cout << "   Port: " << server.listen << std::endl;
        std::cout << "   Nom: " << server.server_name << std::endl;
        std::cout << "   Root: " << server.root << std::endl;
        std::cout << "   Index: " << server.index << std::endl;
        std::cout << "   Taille max body: " << server.client_max_body_size << " bytes" << std::endl;
        std::cout << "   Autoindex: " << (server.autoindex ? "on" : "off") << std::endl;
        
        std::cout << "   Méthodes autorisées: ";
        size_t j = 0;
        while (j < server.allowed_methods.size())
        {
            std::cout << server.allowed_methods[j];
            if (j < server.allowed_methods.size() - 1)
                std::cout << ", ";
            j++;
        }
        std::cout << std::endl;
        
        std::cout << "   Locations (" << server.locations.size() << "):" << std::endl;
        j = 0;
        while (j < server.locations.size())
        {
            const LocationConfig& loc = server.locations[j];
            std::cout << "     • " << loc.path << std::endl;
            std::cout << "       Root: " << (loc.root.empty() ? "(défaut serveur)" : loc.root) << std::endl;
            std::cout << "       Index: " << (loc.index.empty() ? "(défaut serveur)" : loc.index) << std::endl;
            std::cout << "       Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
            j++;
        }
        
        std::cout << std::endl;
        i++;
    }
}