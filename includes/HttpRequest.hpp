#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>
# include <vector>
# include <cstdlib>
// Structure pour stocker une requête HTTP parsée
struct HttpRequest
{
	// Request line
	std::string method;        // GET, POST, DELETE
	std::string uri;           // /path/to/resource (sans query string)
	std::string query_string;  // param1=value1&param2=value2
	std::string version;       // HTTP/1.1
	
	// Headers HTTP (clés en minuscules)
	std::map<std::string, std::string> headers;
	
	// Body de la requête (pour POST)
	std::string body;
	
	// État du parsing
	bool is_valid;             // Requête bien parsée ?
	bool is_complete;          // Requête complètement reçue ?
	int error_code;            // Code d'erreur HTTP si problème
	std::string error_message; // Message d'erreur détaillé
	
	// Constructeur par défaut
	HttpRequest(void) : is_valid(false), is_complete(false), error_code(400) {}
};

// Déclarations des fonctions de parsing
bool ft_parse_request_line(const std::string& line, HttpRequest& request);
bool ft_parse_headers(const std::string& headers_block, HttpRequest& request);
HttpRequest ft_parse_http_request(const std::string& raw_data);
bool ft_is_request_complete(const std::string& data);

// Codes d'erreur HTTP courants
# define HTTP_OK 200
# define HTTP_BAD_REQUEST 400
# define HTTP_NOT_FOUND 404
# define HTTP_METHOD_NOT_ALLOWED 405
# define HTTP_LENGTH_REQUIRED 411
# define HTTP_PAYLOAD_TOO_LARGE 413
# define HTTP_URI_TOO_LONG 414
# define HTTP_VERSION_NOT_SUPPORTED 505

// Limites de sécurité
# define MAX_URI_LENGTH 2048
# define MAX_HEADER_SIZE 8192
# define MAX_HEADERS_COUNT 100
# define MAX_REQUEST_SIZE 1048576  // 1MB

#endif