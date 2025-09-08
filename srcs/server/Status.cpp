/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Status.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 12:07:08 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 12:07:10 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"
#include "../../includes/HttpRequest.hpp"

std::string Server::ft_build_post_success_response(const std::string& message)
{
	// Créer le body HTML
	//CAN BE OTHER THAN 200
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>Success</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>200 - Success!</h1>\n";
	body += "<p>" + message + "</p>\n";
	body += "<p><a href='/'>Retour a l'accueil</a></p>\n";
	body += "<hr>\n<p><em>WebServ/1.0 - " + ft_get_timestamp() + "</em></p>\n";
	body += "</body>\n</html>";
	
	// Calculer Content-Length
	std::ostringstream oss;
	oss << body.length();
	
	// Construire la réponse HTTP
	//CAN BE OTHER THAN 200!!
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

std::string Server::ft_build_404_response(void)
{
	// 1. Créer le body HTML de la page d'erreur 404
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>404 Not Found</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>404 - Page Not Found</h1>\n<p>The requested file was not found.</p>\n";
	body += "<hr>\n<p><em>WebServ/1.0</em></p>\n</body>\n</html>";
	
	// 2. Calculer la taille du body
	std::ostringstream oss;
	oss << body.length();
	
	// 3. Construire la réponse HTTP 404
	std::string response = "HTTP/1.1 404 Not Found\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n\r\n";
	response += body;
	
	return response;
}

std::string Server::ft_build_403_response(void)
{
	// 1. Créer le body HTML de la page d'erreur 403
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>403 Forbidden</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>403 - Forbidden</h1>\n<p>Access denied.</p>\n";
	body += "<hr>\n<p><em>WebServ/1.0</em></p>\n</body>\n</html>";
	
	// 2. Calculer la taille du body
	std::ostringstream oss;
	oss << body.length();
	
	// 3. Construire la réponse HTTP 403
	std::string response = "HTTP/1.1 403 Forbidden\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n\r\n";
	response += body;
	
	return response;
}

std::string Server::ft_build_400_response(void)
{
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>400 Bad Request</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>400 - Bad Request</h1>\n<p>The request could not be understood.</p>\n";
	body += "<hr>\n<p><em>WebServ/1.0</em></p>\n</body>\n</html>";
	
	std::ostringstream oss;
	oss << body.length();
	
	std::string response = "HTTP/1.1 400 Bad Request\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n\r\n";
	response += body;
	
	return response;
}

std::string Server::ft_build_405_response(void)
{
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>405 Method Not Allowed</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>405 - Method Not Allowed</h1>\n<p>The method is not allowed.</p>\n";
	body += "<hr>\n<p><em>WebServ/1.0</em></p>\n</body>\n</html>";
	
	std::ostringstream oss;
	oss << body.length();
	
	std::string response = "HTTP/1.1 405 Method Not Allowed\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n\r\n";
	response += body;
	
	return response;
}

std::string Server::ft_build_413_response(void)
{
	std::string body = "<!DOCTYPE html>\n<html>\n<head><title>413 Payload Too Large</title></head>\n";
	body += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>\n";
	body += "<h1>413 - Payload Too Large</h1>\n<p>The request body is too large.</p>\n";
	body += "<hr>\n<p><em>WebServ/1.0</em></p>\n</body>\n</html>";
	
	std::ostringstream oss;
	oss << body.length();
	
	std::string response = "HTTP/1.1 413 Payload Too Large\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + oss.str() + "\r\n\r\n";
	response += body;
	
	return response;
}
