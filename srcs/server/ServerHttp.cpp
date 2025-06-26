#include "../../includes/Server.hpp"
#include "../../includes/HttpRequest.hpp"
#include <sstream>

HttpRequest ft_parse_http_request(const std::string& raw_data);
bool ft_is_request_complete(const std::string& data);

// NOUVELLE MÉTHODE : Traiter la requête avec la configuration ET le body
std::string Server::ft_handle_request_with_config(const std::string& method, const std::string& uri, const std::string& body)
{
	std::cout << "Processing " << method << " request for: " << uri << " with config" << std::endl;
	
	// 1. Trouver la location correspondante
	const LocationConfig* location = ft_find_location(uri);
	if (location)
		std::cout << "Matched location: " << location->path << " (root: " << location->root << ")" << std::endl;
	else
		std::cout << "No specific location, using server defaults" << std::endl;
	
	// 2. Vérifier si la méthode est autorisée
	if (!ft_is_method_allowed(method, location))
	{
		std::cout << "Method " << method << " not allowed for this location" << std::endl;
		return ft_build_405_response();
	}
	
	if (method == "GET")
	{
		return ft_serve_static_file_with_config(uri);
	}
	else if (method == "POST")
	{
		return ft_handle_post_request_with_config(uri, body);
	}
	else if (method == "DELETE")
	{
		std::cout << "DELETE method (not implemented yet)" << std::endl;
		return ft_build_405_response();
	}
	else
	{
		return ft_build_405_response();
	}
}

// Gérer POST avec configuration
std::string Server::ft_handle_post_request_with_config(const std::string& uri, const std::string& body)
{
	std::cout << "Handling POST request for: " << uri << std::endl;
	std::cout << "POST body: " << body << std::endl;
	
	// 1. Vérifier la taille du body selon la config
	const LocationConfig* location = ft_find_location(uri);
	int max_body_size = location ? location->client_max_body_size : 1000000; // 1MB par défaut
	
	if (static_cast<int>(body.length()) > max_body_size)
	{
		std::cout << "POST body too large: " << body.length() << " > " << max_body_size << std::endl;
		return ft_build_413_response();
	}
	
	// 2. Parser les données POST (comme ton ami)
	std::map<std::string, std::string> post_params = ft_parse_post_data(body);
	
	// 3. Afficher les paramètres reçus
	std::cout << "POST parameters:" << std::endl;
	std::map<std::string, std::string>::iterator it;
	for (it = post_params.begin(); it != post_params.end(); ++it)
	{
		std::cout << "  " << it->first << " = " << it->second << std::endl;
	}
	
	// 4. Router selon l'URI
	if (uri == "/login")
	{
		return ft_handle_login(post_params);
	}
	else if (uri == "/upload")
	{
		return ft_handle_upload(post_params);
	}
	else if (uri == "/contact")
	{
		return ft_handle_contact(post_params);
	}
	else
	{
		return ft_build_post_success_response("POST reçu avec succès pour " + uri + " !");
	}
}

// Servir fichier statique avec la config
std::string Server::ft_serve_static_file_with_config(const std::string& uri)
{
	std::cout << "Serving static file for URI: " << uri << " with config" << std::endl;
	
	// 1. Trouver la location correspondante
	const LocationConfig* location = ft_find_location(uri);
	
	// 2. Construire le chemin du fichier selon la config
	std::string file_path = ft_get_file_path(uri, location);
	
	std::cout << "File path resolved to: " << file_path << std::endl;
	
	// 3. Vérifier si c'est un script CGI
	if (file_path.length() > 3 && file_path.substr(file_path.length() - 3) == ".py")
	{
		std::cout << "Executing CGI script: " << file_path << std::endl;
		return ft_execute_cgi(file_path);
	}
	
	// 4. Lire le contenu du fichier
	std::string file_content = ft_read_file_simple(file_path);
	
	// 5. Vérifier si le fichier existe
	if (file_content.empty())
	{
		std::cout << "File not found: " << file_path << std::endl;
		return ft_build_404_response();
	}
	
	// 6. Déterminer le Content-Type selon l'extension
	std::string content_type = ft_get_content_type(file_path);
	
	// 7. Calculer Content-Length
	std::ostringstream oss;
	oss << file_content.length();
	
	// 8. Construire la réponse HTTP 200 OK
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: " + content_type + "\r\n";
	response += "Content-Length: " + oss.str() + "\r\n";
	response += "Server: WebServ/1.0\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += file_content;
	
	return response;
}

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

// NOUVELLES MÉTHODES pour les erreurs de parsing HTTP

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