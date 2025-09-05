/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerNetwork.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 11:52:29 by jmaizel           #+#    #+#             */
/*   Updated: 2025/07/23 11:52:30 by jmaizel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"
#include "../../includes/HttpRequest.hpp"
#include <sstream>

// Fonctions de parsing HTTP externes
HttpRequest ft_parse_http_request(const std::string& raw_data);
bool ft_is_request_complete(const std::string& data);

void Server::ft_accept_new_client(void)
{
	// 1. Préparer la structure pour l'adresse du client
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	// 2. Accepter la connexion
	int client_fd = accept(_server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
	{
		// Pas d'erreur grave, juste pas de client maintenant
		return;
	}

	// 3. Rendre le socket client non-bloquant aussi
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(client_fd);  // Fermer si on peut pas configurer
		return;
	}

	// 4. Ajouter le nouveau client à notre surveillance
	FD_SET(client_fd, &_master_fds);
	_client_fds.push_back(client_fd);
	
	// 5. Mettre à jour le max_fd si nécessaire
	if (client_fd > _max_fd)
		_max_fd = client_fd;

	std::cout << "New client connected: " << client_fd << std::endl;
}

std::string Server::ft_handle_delete(const std::string& uri) {
    // 1. Convertir l'URI en chemin local (à adapter selon ton root)
    std::string path = "./" + uri;

    // 2. Vérifier si le fichier existe
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
	{
		if (errno == EACCES)
        	return ft_build_403_response(); 	
		else if (errno == ENOENT)
        	return ft_build_404_response();
		else
		{
        	return ft_build_500_response();
    	}
    }

    // 3. Vérifier que c'est un fichier (pas un dossier)
    if (S_ISDIR(st.st_mode))
	{
        return ft_build_403_response();
    }

    // 4. Essayer de supprimer le fichier
	//CAN I USE REMOVE/UNLINK?
    if (remove(path.c_str()) == 0)
	{
		//DONT KNOW WHAT TO PUT
        return ft_build_success_response("was removed", " yayyy");
    } 
	else
	{
        return ft_build_500_response();
    }
}

void Server::ft_handle_client_request(int client_fd)
{
	// 1. Buffer pour recevoir les données
	char buffer[BUFFER_SIZE];
	ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
	
	// 2. Vérifier si le client s'est déconnecté
	if (bytes_read <= 0)
	{
		// Client déconnecté ou erreur
		ft_disconnect_client(client_fd);
		return;
	}

	// 3. Terminer la chaîne reçue
	buffer[bytes_read] = '\0';
	std::string raw_data(buffer);
	
	std::cout << "Raw data received (" << bytes_read << " bytes)" << std::endl;
	
	// 4. Vérifier si la requête est complète
	if (!ft_is_request_complete(raw_data))
	{
		std::cout << "Incomplete request, waiting for more data..." << std::endl;
		return;
	}
	
	std::cout << "Complete request received, parsing..." << std::endl;
	
	// 5. Parser la requête HTTP complète
	HttpRequest request = ft_parse_http_request(raw_data);
	
	// 6. Vérifier si le parsing a réussi
	if (!request.is_valid)
	{
		std::cout << "Parse error: " << request.error_message << std::endl;
		std::string error_response = ft_build_400_response();
		send(client_fd, error_response.c_str(), error_response.length(), 0);
		ft_disconnect_client(client_fd);
		return;
	}
	
	// 7. Afficher les infos de la requête parsée
	std::cout << "✓ Request parsed successfully:" << std::endl;
	std::cout << "  Method: " << request.method << std::endl;
	std::cout << "  URI: " << request.uri << std::endl;
	std::cout << "  Version: " << request.version << std::endl;
	std::cout << "  Headers: " << request.headers.size() << " header(s)" << std::endl;
	
	// 8. NOUVEAU : Utiliser la configuration pour traiter la requête avec le body
	std::string response = ft_handle_request_with_config(request.method, request.uri, request.body);
	
	// 9. Envoyer la réponse au client
	send(client_fd, response.c_str(), response.length(), 0);
	
	// 10. Fermer la connexion
	ft_disconnect_client(client_fd);
}

void Server::ft_disconnect_client(int client_fd)
{
	std::cout << "Client disconnected: " << client_fd << std::endl;
	
	// Fermer le socket
	close(client_fd);
	
	// Retirer de la surveillance select()
	FD_CLR(client_fd, &_master_fds);
	
	// Retirer de notre liste de clients
	size_t i = 0;
	while (i < _client_fds.size())
	{
		if (_client_fds[i] == client_fd)
		{
			_client_fds.erase(_client_fds.begin() + i);
			break;
		}
		i++;
	}
}
