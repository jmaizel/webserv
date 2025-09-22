/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:41:30 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/22 00:00:00 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

// Fonction pour vérifier si un fichier est un script CGI
bool ft_is_cgi_script(const std::string& file_path)
{
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos == std::string::npos)
        return false;
        
    std::string ext = file_path.substr(dot_pos);
    return (ext == ".py" || ext == ".php" || ext == ".pl" || ext == ".sh");
}

// Fonction pour extraire la query string de l'URI
std::string ft_extract_query_string(const std::string& uri)
{
    size_t query_pos = uri.find('?');
    if (query_pos != std::string::npos)
        return uri.substr(query_pos + 1);
    return "";
}

// Fonction pour déterminer l'interpréteur selon l'extension
std::string ft_get_interpreter(const std::string& script_path)
{
    size_t dot_pos = script_path.find_last_of('.');
    if (dot_pos != std::string::npos)
    {
        std::string ext = script_path.substr(dot_pos);
        if (ext == ".py")
            return "python3";
        else if (ext == ".php")
            return "php";
        else if (ext == ".pl")
            return "perl";
        else if (ext == ".sh")
            return "sh";
    }
    return "python3";
}

// Configuration des variables d'environnement CGI
void ft_setup_cgi_environment(const HttpRequest& request, const std::string& server_name,
                              int server_port, const std::string& script_path)
{
    setenv("REQUEST_METHOD", request.getMethod().c_str(), 1);
    
    std::string query_string = ft_extract_query_string(request.getTarget());
    setenv("QUERY_STRING", query_string.c_str(), 1);
    
    setenv("SERVER_NAME", server_name.c_str(), 1);
    
    std::ostringstream port_stream;
    port_stream << server_port;
    setenv("SERVER_PORT", port_stream.str().c_str(), 1);
    
    setenv("CONTENT_LENGTH", "0", 1);
    setenv("SCRIPT_NAME", script_path.c_str(), 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
}

// Processus enfant pour l'exécution CGI
void ft_execute_cgi_child(const std::string& script_path, const HttpRequest& request,
                          const std::string& server_name, int server_port,
                          int pipe_in[2], int pipe_out[2])
{
    dup2(pipe_in[0], STDIN_FILENO);
    dup2(pipe_out[1], STDOUT_FILENO);
    
    close(pipe_in[0]); 
    close(pipe_in[1]);
    close(pipe_out[0]); 
    close(pipe_out[1]);
    
    chdir("./www/");
    
    ft_setup_cgi_environment(request, server_name, server_port, script_path);
    
    std::string interpreter = ft_get_interpreter(script_path);
    std::string script_name;
if (script_path.find("./www/") == 0)
{
    script_name = script_path.substr(6);
}
else
{
    script_name = script_path;
}
    
    char *args[] = {const_cast<char*>(interpreter.c_str()), 
                    const_cast<char*>(script_name.c_str()), NULL};
    execvp(interpreter.c_str(), args);
    
    exit(1);
}

// Fonction pour parser les headers CGI et créer la réponse
HttpResponse ft_parse_cgi_headers(const std::string& headers_part, const std::string& body_part)
{
    HttpResponse response;
    std::istringstream header_stream(headers_part);
    std::string header_line;
    bool has_content_type = false;
    
    while (std::getline(header_stream, header_line))
    {
        if (header_line.empty() || header_line == "\r")
            continue;
            
        size_t colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos)
        {
            std::string header_name = header_line.substr(0, colon_pos);
            std::string header_value = header_line.substr(colon_pos + 1);
            
            // Nettoyer les espaces
            while (!header_value.empty() && (header_value[0] == ' ' || header_value[0] == '\t'))
                header_value.erase(0, 1);
            while (!header_value.empty() && (header_value.back() == '\r' || header_value.back() == '\n'))
                header_value.pop_back();
            
            response.setHeaders(header_name, header_value);
            
            if (header_name == "Content-Type")
                has_content_type = true;
        }
    }
    
    // Ajouter Content-Type par défaut si absent
    if (!has_content_type)
        response.setHeaders("Content-Type", "text/html");
    
    // Définir Content-Length
    std::ostringstream length_stream;
    length_stream << body_part.length();
    response.setHeaders("Content-Length", length_stream.str());
    
    // Autres headers standards
    response.setHeaders("Server", "WebServ/1.0");
    response.setHeaders("Connection", "close");
    
    return response;
}

// Fonction pour construire une réponse HTTP à partir de la sortie CGI
HttpResponse ft_build_cgi_response(const std::string& cgi_output)
{
    HttpResponse response;
    
    // Séparer headers et body
    size_t header_end = cgi_output.find("\r\n\r\n");
    if (header_end == std::string::npos)
        header_end = cgi_output.find("\n\n");
    
    std::string headers_part;
    std::string body_part;
    
    if (header_end != std::string::npos)
    {
        headers_part = cgi_output.substr(0, header_end);
        if (cgi_output.find("\r\n\r\n") != std::string::npos)
            body_part = cgi_output.substr(header_end + 4);
        else
            body_part = cgi_output.substr(header_end + 2);
    }
    else
    {
        body_part = cgi_output;
        headers_part = "Content-Type: text/html";
    }
    
    // Configurer la réponse de base
    response.setVersion("HTTP/1.1");
    response.setStatusCode(200);
    response.setReason("OK");
    
    // Parser les headers CGI
    response = ft_parse_cgi_headers(headers_part, body_part);
    
    // Définir le body
    response.setBody(body_part);
    
    return response;
}

// Fonction pour créer une réponse d'erreur CGI
HttpResponse ft_create_cgi_error(int code, const std::string& reason, const std::string& message)
{
    HttpResponse error_response;
    error_response.setVersion("HTTP/1.1");
    error_response.setStatusCode(code);
    error_response.setReason(reason);
    
    std::string body = "<html><body><h1>" + std::to_string(code) + " " + reason + "</h1><p>" + message + "</p></body></html>";
    error_response.setBody(body);
    
    std::ostringstream length_stream;
    length_stream << body.length();
    error_response.setHeaders("Content-Length", length_stream.str());
    error_response.setHeaders("Content-Type", "text/html");
    error_response.setHeaders("Connection", "close");
    
    return error_response;
}

// Fonction principale pour exécuter un script CGI - retourne HttpResponse
HttpResponse ft_execute_cgi(const std::string& script_path, const HttpRequest& request, 
                           const std::string& server_name, int server_port)
{
    int pipe_in[2], pipe_out[2];
    
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        return ft_create_cgi_error(500, "Internal Server Error", "Failed to create pipes for CGI execution");
    }
    
    pid_t pid = fork();
    if (pid == -1)
    {
        close(pipe_in[0]); 
        close(pipe_in[1]);
        close(pipe_out[0]); 
        close(pipe_out[1]);
        return ft_create_cgi_error(500, "Internal Server Error", "Failed to fork process for CGI execution");
    }
    
    if (pid == 0)
    {
        ft_execute_cgi_child(script_path, request, server_name, server_port, pipe_in, pipe_out);
    }
    
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[1]);
    
    std::string cgi_output;
    char buffer[4096];
    ssize_t bytes_read;
    
    while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        cgi_output += buffer;
    }
    
    close(pipe_out[0]);
    
    int status;
    waitpid(pid, &status, 0);
    
    if (WEXITSTATUS(status) != 0)
    {
        return ft_create_cgi_error(500, "Internal Server Error", "CGI script execution failed");
    }
    
    return ft_build_cgi_response(cgi_output);
}