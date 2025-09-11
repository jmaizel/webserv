/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerMonitor.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 17:49:05 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/11 17:30:00 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_MONITOR_HPP
# define SERVER_MONITOR_HPP

#include "Server.hpp"

class ServerMonitor
{
    private:
        std::vector<Server> _servers;
        std::string         _config;

    public:
        ServerMonitor();
        ServerMonitor(std::string &config);
        ~ServerMonitor();

        // TES méthodes existantes
        void    print();
        void    addServer(Server server);
        void    run();
        void    parse();
        
        // Nouvelle méthode pour initialiser tous les serveurs
        void    init_servers();
};

// Fonctions de parsing
LocationBloc get_location_bloc(std::vector<std::string> &tokens, std::string &pathi, size_t *i);
ServerBloc get_server_bloc(std::vector<std::string> &tokens, size_t *i);

#endif