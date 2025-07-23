/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerHttpPost.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 11:52:23 by jmaizel           #+#    #+#             */
/*   Updated: 2025/07/23 11:52:24 by jmaizel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"
#include <sstream>
#include <ctime>

// Parser les données POST (application/x-www-form-urlencoded)
std::map<std::string, std::string> Server::ft_parse_post_data(const std::string& body)
{
	std::map<std::string, std::string> params;
	
	if (body.empty())
		return params;
	
	// Exemple de body : "username=john&password=secret&email=john@test.com"
	std::string current_body = body;
	
	// Parcourir les paramètres séparés par '&'
	size_t amp_pos = 0;
	while (amp_pos != std::string::npos)
	{
		// Trouver le prochain '&' ou la fin
		size_t next_amp = current_body.find('&', amp_pos);
		
		// Extraire une paire key=value
		std::string pair;
		if (next_amp != std::string::npos)
		{
			pair = current_body.substr(amp_pos, next_amp - amp_pos);
			amp_pos = next_amp + 1;
		}
		else
		{
			pair = current_body.substr(amp_pos);
			amp_pos = std::string::npos;  // Dernière itération
		}
		
		// Séparer key=value
		size_t equal_pos = pair.find('=');
		if (equal_pos != std::string::npos)
		{
			std::string key = pair.substr(0, equal_pos);
			std::string value = pair.substr(equal_pos + 1);
			params[key] = value;
		}
	}
	
	return params;
}

// Gérer la connexion (login)
std::string Server::ft_handle_login(const std::map<std::string, std::string>& params)
{
	std::map<std::string, std::string>::const_iterator user_it = params.find("username");
	std::map<std::string, std::string>::const_iterator pass_it = params.find("password");
	
	if (user_it == params.end() || pass_it == params.end())
	{
		return ft_build_400_response();
	}
	
	std::string username = user_it->second;
	std::string password = pass_it->second;
	
	// Authentification avec les identifiants de tes pages HTML
	if (username == "hsorel" && password == "qwerty")
	{
		// Redirection vers dev.html (connexion réussie)
		return ft_build_redirect_response("/dev.html");
	}
	else
	{
		// Redirection vers invalidlogin.html (échec)
		return ft_build_redirect_response("/invalidlogin.html");
	}
}

// Gérer l'upload de fichiers
std::string Server::ft_handle_upload(const std::map<std::string, std::string>& params)
{
	std::map<std::string, std::string>::const_iterator file_it = params.find("filename");
	std::map<std::string, std::string>::const_iterator content_it = params.find("content");
	
	if (file_it == params.end())
	{
		return ft_build_post_success_response("Upload simulé - aucun fichier spécifié");
	}
	
	std::string filename = file_it->second;
	std::string content = (content_it != params.end()) ? content_it->second : "contenu vide";
	
	std::string success_msg = "Fichier '" + filename + "' uploadé avec succès (" + content + ")";
	return ft_build_post_success_response(success_msg);
}

// Gérer le formulaire de contact
std::string Server::ft_handle_contact(const std::map<std::string, std::string>& params)
{
	std::map<std::string, std::string>::const_iterator name_it = params.find("name");
	std::map<std::string, std::string>::const_iterator email_it = params.find("email");
	std::map<std::string, std::string>::const_iterator message_it = params.find("message");
	
	std::string name = (name_it != params.end()) ? name_it->second : "Anonyme";
	std::string email = (email_it != params.end()) ? email_it->second : "non spécifié";
	std::string message = (message_it != params.end()) ? message_it->second : "message vide";
	
	std::string response_msg = "Message reçu de " + name + " (" + email + "): " + message;
	return ft_build_post_success_response(response_msg);
}

// Construire une réponse de succès pour POST
std::string Server::ft_build_post_success_response(const std::string& message)
{
	// Créer le body HTML
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>Success</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>✓ Succès</h1>\n";
	body += "<p>" + message + "</p>\n";
	body += "<p><a href='/'>Retour à l'accueil</a></p>\n";
	body += "<hr>\n<p><em>WebServ/1.0 - " + ft_get_timestamp() + "</em></p>\n";
	body += "</body>\n</html>";
	
	// Calculer Content-Length
	std::ostringstream oss;
	oss << body.length();
	
	// Construire la réponse HTTP
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n";
	response += "Server: WebServ/1.0\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += body;
	
	return response;
}

// Construire une réponse 401 Unauthorized
std::string Server::ft_build_401_response(const std::string& content)
{
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>401 Unauthorized</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>401 - Non autorisé</h1>\n";
	body += "<p>" + content + "</p>\n";
	body += "<hr>\n<p><em>WebServ/1.0</em></p>\n</body>\n</html>";
	
	std::ostringstream oss;
	oss << body.length();
	
	std::string response = "HTTP/1.1 401 Unauthorized\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n\r\n";
	response += body;
	
	return response;
}

// Construire une réponse 500 Internal Server Error
std::string Server::ft_build_500_response(void)
{
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>500 Internal Server Error</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>500 - Erreur interne du serveur</h1>\n";
	body += "<p>Une erreur s'est produite lors du traitement de votre requête.</p>\n";
	body += "<hr>\n<p><em>WebServ/1.0</em></p>\n</body>\n</html>";
	
	std::ostringstream oss;
	oss << body.length();
	
	std::string response = "HTTP/1.1 500 Internal Server Error\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n\r\n";
	response += body;
	
	return response;
}

// Construire une réponse de succès générique
std::string Server::ft_build_success_response(const std::string& content, const std::string& content_type)
{
	std::ostringstream oss;
	oss << content.length();
	
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: " + content_type + "\r\n";
	response += "Content-Length: " + oss.str() + "\r\n";
	response += "Server: WebServ/1.0\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += content;
	
	return response;
}

// Obtenir un timestamp pour les logs
std::string Server::ft_get_timestamp(void)
{
	time_t raw_time;
	struct tm* time_info;
	char buffer[80];
	
	time(&raw_time);
	time_info = localtime(&raw_time);
	
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);
	return std::string(buffer);
}

// Construire une réponse de redirection HTTP 302
std::string Server::ft_build_redirect_response(const std::string& location)
{
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>Redirection</title></head>\n";
	body += "<body>\n<p>Redirection en cours vers <a href=\"" + location + "\">" + location + "</a></p>\n";
	body += "</body>\n</html>";
	
	std::ostringstream oss;
	oss << body.length();
	
	std::string response = "HTTP/1.1 302 Found\r\n";
	response += "Location: " + location + "\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n";
	response += "Server: WebServ/1.0\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += body;
	
	return response;
}
