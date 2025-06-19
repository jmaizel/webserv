#include "HttpRequest.hpp"
#include <sstream>
#include <iostream>

// Parser la première ligne d'une requête HTTP
// Format : "GET /path HTTP/1.1"
bool ft_parse_request_line(const std::string& line, HttpRequest& request)
{
	if (line.empty())
	{
		request.error_message = "Empty request line";
		return false;
	}
	
	// Utiliser istringstream pour séparer les mots
	std::istringstream iss(line);
	std::string method, uri, version;
	
	// Extraire les 3 parties
	if (!(iss >> method >> uri >> version))
	{
		request.error_message = "Malformed request line";
		return false;
	}
	
	// Vérifications basiques
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		request.error_message = "Method not supported: " + method;
		return false;
	}
	
	if (version != "HTTP/1.1")
	{
		request.error_message = "Version not supported: " + version;
		return false;
	}
	
	if (uri.empty() || uri[0] != '/')
	{
		request.error_message = "Invalid URI: " + uri;
		return false;
	}
	
	// Tout est OK, stocker les valeurs
	request.method = method;
	request.uri = uri;
	request.version = version;
	
	return true;
}