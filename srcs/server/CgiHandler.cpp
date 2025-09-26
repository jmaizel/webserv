/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 20:10:47 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/11 20:10:48 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

//vérifier si un fichier est un script CGI
bool ft_is_cgi_script(const std::string& file_path)
{
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos == std::string::npos)
        return false;
        
    std::string ext = file_path.substr(dot_pos);
    return (ext == ".py" || ext == ".php" || ext == ".pl" || ext == ".sh");
}

//extraire la query string de l'URI
std::string ft_extract_query_string(const std::string& uri)
{
    size_t query_pos = uri.find('?');
    if (query_pos != std::string::npos)
        return uri.substr(query_pos + 1);
    return "";
}

//déterminer l'interpréteur selon l'extension
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

//configuration des variables d'environnement CGI
void ft_setup_cgi_environment(const HttpRequest& request, const std::string& server_name, int server_port, const std::string& script_path)
{
    setenv("REQUEST_METHOD", request.getMethod().c_str(), 1);
    
    std::string query_string = ft_extract_query_string(request.getTarget());
    setenv("QUERY_STRING", query_string.c_str(), 1);
    
    setenv("SERVER_NAME", server_name.c_str(), 1);
    
    std::ostringstream port_stream;
    port_stream << server_port;
    setenv("SERVER_PORT", port_stream.str().c_str(), 1);
    
    std::ostringstream length_stream;
    length_stream << request.getBody().length();
    setenv("CONTENT_LENGTH", length_stream.str().c_str(), 1);

    //si POST, transmettre aussi le Content-Type
    std::map<std::string, std::string> headers = request.getHeaders();
    if (request.getMethod() == "POST" && headers.count("Content-Type"))
        setenv("CONTENT_TYPE", headers["Content-Type"].c_str(), 1);
    setenv("SCRIPT_NAME", script_path.c_str(), 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
}

//processus enfant pour l'exécution CGI
void ft_execute_cgi_child(const std::string& script_path, const HttpRequest& request, const std::string& server_name, int server_port,int pipe_in[2], int pipe_out[2])
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

//parser les headers CGI et créer la réponse
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
            
            //trim les espaces
            while (!header_value.empty() && (header_value[0] == ' ' || header_value[0] == '\t'))
                header_value.erase(0, 1);
           while (!header_value.empty() && (header_value[header_value.size() - 1] == '\r' || header_value[header_value.size() - 1] == '\n'))
            {
                header_value.erase(header_value.size() - 1, 1);
            }
            response.setHeaders(header_name, header_value);
            
            if (header_name == "Content-Type")
                has_content_type = true;
        }
    }
    
    //ajouter Content-Type par défaut si absent
    if (!has_content_type)
        response.setHeaders("Content-Type", "text/html");
    
    //définir Content-Length
    std::ostringstream length_stream;
    length_stream << body_part.length();
    response.setHeaders("Content-Length", length_stream.str());
    
    //autres headers standards
    response.setHeaders("Server", "WebServ/1.0");
    response.setHeaders("Connection", "close");
    return response;
}

//fonction pour construire une réponse HTTP à partir de la sortie CGI
HttpResponse ft_build_cgi_response(const std::string& cgi_output)
{
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

    //correction : parser les headers et les ajouter à response, sans écraser la version/status
    HttpResponse response;
    response.setVersion("HTTP/1.1");
    response.setStatusCode(200);
    response.setReason("OK");

    HttpResponse parsed = ft_parse_cgi_headers(headers_part, body_part);
    //utilisation de la méthode getHeaders()
    std::map<std::string, std::string> hdrs = parsed.getHeaders();
    for (std::map<std::string, std::string>::iterator it = hdrs.begin(); it != hdrs.end(); ++it)
        response.setHeaders(it->first, it->second);
    //définir le body
    response.setBody(body_part);
    return response;
}

//fonction pour créer une réponse d'erreur CGI
HttpResponse ft_create_cgi_error(int code, const std::string& reason, const std::string& message)
{
    HttpResponse error_response;
    error_response.setVersion("HTTP/1.1");
    error_response.setStatusCode(code);
    error_response.setReason(reason);
    
    std::string body = "<html><body><h1>" + to_string98(code) + " " + reason + "</h1><p>" + message + "</p></body></html>";
    error_response.setBody(body);
    
    std::ostringstream length_stream;
    length_stream << body.length();
    error_response.setHeaders("Content-Length", length_stream.str());
    error_response.setHeaders("Content-Type", "text/html");
    error_response.setHeaders("Connection", "close");
    
    return error_response;
}

//fonction principale pour exécuter un script CGI - retourne HttpResponse
HttpResponse Server::generate_cgi_response(const std::string& script_path, const HttpRequest& request, LocationBloc &location)
{
    //check permissions
    if (access(script_path.c_str(), X_OK) < 0)
    {
        return generate_error_response(403, "Forbidden", "You do not have to necessary permissions", location);
    }

    int pipe_in[2], pipe_out[2];

    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
        return generate_error_response(403, "Forbidden", "Pipe failed", location);

    pid_t pid = fork();
    if (pid == -1)
    {
        close(pipe_in[0]); 
        close(pipe_in[1]);
        close(pipe_out[0]); 
        close(pipe_out[1]);
        return generate_error_response(403, "Forbidden", "Fork failed", location);
    }

    if (pid == 0)
    {
        ft_execute_cgi_child(script_path, request, this->_name, this->_listen, pipe_in, pipe_out);
    }

    //parent
    close(pipe_in[0]);
    close(pipe_out[1]);

    //si POST, écrire le body dans le pipe
    if (request.getMethod() == "POST" && !request.getBody().empty())
    {
        write(pipe_in[1], request.getBody().c_str(), request.getBody().length());
    }
    close(pipe_in[1]);

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
        return generate_error_response(403, "Forbidden", "CGI execution failed", location);

    //NO ADD LOCATION
    return ft_build_cgi_response(cgi_output);
}