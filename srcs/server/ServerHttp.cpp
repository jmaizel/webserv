/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerHttp.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 11:52:20 by jmaizel           #+#    #+#             */
/*   Updated: 2025/07/23 12:01:12 by jmaizel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"
#include "../../includes/HttpRequest.hpp"
#include <sstream>

HttpRequest ft_parse_http_request(const std::string& raw_data);
bool ft_is_request_complete(const std::string& data);

// Traiter la requête avec la configuration ET le body
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
	/* NOT needed! DELETE is diguised as POST
	else if (method == "DELETE")
	{
		return ft_handle_delete(uri);
	}
	*/
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
	
	// 2. Parser les données POST
	std::map<std::string, std::string> post_params = ft_parse_post_data(body);
	
	// 3. Afficher les paramètres reçus
	std::cout << "POST parameters:" << std::endl;
	std::map<std::string, std::string>::iterator it;
	for (it = post_params.begin(); it != post_params.end(); ++it)
	{
		std::cout << "  " << it->first << " = " << it->second << std::endl;
	}
	
	//DELETE functions is disguised as a POST
	if (post_params.find("_method") != post_params.end() && post_params["_method"] == "DELETE")
	{
		//deletes the file pointed by uri
    	return ft_handle_delete(uri, post_params);
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
		
    	// Créer un HttpRequest basique pour le CGI
    	HttpRequest cgi_request;
    	cgi_request.method = "GET";
    	cgi_request.uri = uri;
    	cgi_request.version = "HTTP/1.1";
    	cgi_request.is_valid = true;
		
    	// Ajouter la query string si elle existe
    	size_t query_pos = uri.find('?');
    	if (query_pos != std::string::npos)
    	{
    	    cgi_request.query_string = uri.substr(query_pos + 1);
    	}
	
    	return ft_execute_cgi(file_path, cgi_request);
	}
	
	// 4. Lire le contenu du fichier
	std::string file_content = ft_read_file_simple(file_path);
	
	// 5. Vérifier si le fichier existe
	if (file_content.empty())
	{
		if (errno == EACCES || errno == EISDIR)
        	return ft_build_403_response(); 	
		else if (errno == ENOENT)
        	return ft_build_404_response();
		else
		{
        	return ft_build_500_response();
    	}
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
    
    // Créer un HttpRequest basique
    HttpRequest cgi_request;
    cgi_request.method = "GET";
    cgi_request.uri = uri;
    cgi_request.version = "HTTP/1.1";
    cgi_request.is_valid = true;
    
    // Ajouter la query string si elle existe
    size_t query_pos = uri.find('?');
    if (query_pos != std::string::npos)
    {
        cgi_request.query_string = uri.substr(query_pos + 1);
    }
    
    return ft_execute_cgi(script_path, cgi_request);
}
	
	// 2. Sinon → fichier statique (HTML, CSS, JS, images...)
	return ft_serve_static_file(uri);
}

std::string Server::ft_get_directory_path(const std::string& file_path)
{
    // Trouver la dernière occurrence de '/'
    size_t last_slash = file_path.find_last_of('/');
    
    if (last_slash == std::string::npos)
    {
        // Pas de '/' trouvé, le fichier est dans le répertoire courant
        return "./";
    }
    
    if (last_slash == 0)
    {
        // Le '/' est au début, c'est la racine
        return "/";
    }
    
    // Retourner tout ce qui précède le dernier '/'
    return file_path.substr(0, last_slash);
}

std::string Server::ft_execute_cgi(const std::string& script_path, const HttpRequest& request)
{
    int pipe_in[2], pipe_out[2];
    
    // Créer les pipes pour communiquer avec le script
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
        return ft_build_500_response();
    
    pid_t pid = fork();
    if (pid == -1)
    {
        close(pipe_in[0]); close(pipe_in[1]);
        close(pipe_out[0]); close(pipe_out[1]);
        return ft_build_500_response();
    }
    
    if (pid == 0) // Processus enfant = le script CGI
    {
        // Rediriger stdin et stdout vers les pipes
        dup2(pipe_in[0], STDIN_FILENO);   // Le script lira depuis pipe_in
        dup2(pipe_out[1], STDOUT_FILENO); // Le script écrira vers pipe_out
        
        // Fermer les descripteurs inutiles
        close(pipe_in[0]); close(pipe_in[1]);
        close(pipe_out[0]); close(pipe_out[1]);
        
        // Définir les variables d'environnement CGI
        setenv("REQUEST_METHOD", request.method.c_str(), 1);
        
        if (!request.query_string.empty())
            setenv("QUERY_STRING", request.query_string.c_str(), 1);
        else
            setenv("QUERY_STRING", "", 1);
        
        if (request.method == "POST")
        {
            std::ostringstream oss;
            oss << request.body.length();
            setenv("CONTENT_LENGTH", oss.str().c_str(), 1);
            
            std::map<std::string, std::string>::const_iterator it;
            it = request.headers.find("content-type");
            if (it != request.headers.end())
                setenv("CONTENT_TYPE", it->second.c_str(), 1);
            else
                setenv("CONTENT_TYPE", "application/x-www-form-urlencoded", 1);
        }
        else
        {
            setenv("CONTENT_LENGTH", "0", 1);
        }
        
       setenv("SCRIPT_NAME", script_path.c_str(), 1);
setenv("SERVER_NAME", _config.server_name.c_str(), 1);

std::ostringstream port_oss;
port_oss << _config.listen;
setenv("SERVER_PORT", port_oss.str().c_str(), 1);

setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);

// Ajouter tous les headers HTTP comme HTTP_*
std::map<std::string, std::string>::const_iterator it;
for (it = request.headers.begin(); it != request.headers.end(); ++it)
{
    std::string env_name = "HTTP_" + ft_to_upper_env(it->first);
    setenv(env_name.c_str(), it->second.c_str(), 1);
}

// Gestion correcte des chemins
chdir("./www");

// Calculer le nom du script relatif au dossier www
std::string script_name;
if (script_path.find("./www/") == 0)
{
    script_name = script_path.substr(6); // Enlever "./www/"
}
else
{
    script_name = script_path;
}

// Exécuter le script Python
char* args[] = {(char*)"python3", (char*)script_name.c_str(), NULL};
execve("/usr/bin/python3", args, environ);
        
        // Si on arrive ici, execve a échoué
        exit(1);
    }
    else // Processus parent = ton serveur
    {
        // Fermer les côtés inutiles des pipes
        close(pipe_in[0]);
        close(pipe_out[1]);
        
        // Si c'est POST, envoyer le body au script via stdin
        if (request.method == "POST" && !request.body.empty())
        {
            write(pipe_in[1], request.body.c_str(), request.body.length());
        }
        close(pipe_in[1]); // Fermer stdin (EOF pour le script)
        
        // Lire la réponse du script depuis stdout
        std::string cgi_output;
        char buffer[1024];
        ssize_t bytes_read;
        
        while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytes_read] = '\0';
            cgi_output += buffer;
        }
        close(pipe_out[0]);
        
        // Attendre que le processus enfant se termine
        int status;
        waitpid(pid, &status, 0);
        
        // Vérifier si le script s'est terminé correctement
        if (WEXITSTATUS(status) != 0)
            return ft_build_500_response();
        
        // Parser la sortie CGI et construire la réponse HTTP
        return ft_build_cgi_response(cgi_output);
    }
    
    return ft_build_500_response(); // Ne devrait jamais arriver
}

std::string Server::ft_build_cgi_response(const std::string& cgi_output)
{
    // Le script CGI renvoie : headers + ligne vide + body
    size_t separator = cgi_output.find("\n\n");
    if (separator == std::string::npos)
        separator = cgi_output.find("\r\n\r\n");
    
    if (separator == std::string::npos)
        return ft_build_500_response();
    
    std::string cgi_headers = cgi_output.substr(0, separator);
    std::string cgi_body = cgi_output.substr(separator + 2);
    
    // Construire la réponse HTTP complète
    std::ostringstream oss;
    oss << cgi_body.length();
    
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += cgi_headers + "\r\n"; // Headers du script
    response += "Content-Length: " + oss.str() + "\r\n";
    response += "Server: WebServ/1.0\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += cgi_body;
    
    return response;
}

// Fonction helper pour convertir les noms de headers
std::string Server::ft_to_upper_env(const std::string& str)
{
    std::string result = str;
    size_t i = 0;
    while (i < result.length())
    {
        if (result[i] == '-')
            result[i] = '_';
        else
            result[i] = std::toupper(result[i]);
        i++;
    }
    return result;
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
