#include "Server.hpp"
#include <sstream>

std::string Server::ft_handle_request_simple(const std::string& uri)
{
	// 1. Décision : script CGI ou fichier statique ?
	if (uri.length() > 3 && uri.substr(uri.length() - 3) == ".py")
	{
		// C'est un script Python → exécuter via CGI
		std::string script_path = "./www" + uri;
		return ft_execute_cgi(script_path);
	}
	
	// 2. Sinon → fichier statique (HTML, CSS, JS, images...)
	return ft_serve_static_file(uri);
}

std::string Server::ft_execute_cgi(const std::string& script_path)
{
	// 1. Construire la commande à exécuter
	std::string command = "python3 " + script_path;
	
	// 2. Exécuter le script et capturer sa sortie
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe)
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 13\r\n\r\nScript failed";
	
	// 3. Lire la sortie du script ligne par ligne
	std::string result;
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), pipe) != NULL)
	{
		result += buffer;
	}
	pclose(pipe);
	
	// 4. Séparer headers et body du script
	size_t body_start = result.find("\r\n\r\n");
	if (body_start == std::string::npos)
		return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 17\r\n\r\nMalformed script";
	
	std::string script_headers = result.substr(0, body_start + 2); // Garde \r\n
	std::string body = result.substr(body_start + 4); // Skip \r\n\r\n
	
	// 5. Calculer Content-Length du body
	std::ostringstream oss;
	oss << body.length();
	
	// 6. Construire la réponse HTTP complète
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += script_headers;   // Headers du script (Content-Type, etc.)
	response += "Content-Length: " + oss.str() + "\r\n";
	response += "\r\n";           // Ligne vide obligatoire
	response += body;             // Contenu généré par le script
	
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