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


//parses special characters to get a predicatble output
#include <string>
#include <cctype>
#include <stdexcept>

//Decode a single hex digit
static int fromHex(char c) 
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	else 
	{
		return c;
	}
}

std::string bodyDecode(const std::string &src)
{
    std::string result;
    result.reserve(src.size());

    for (size_t i = 0; i < src.size(); ++i)
	{
        char c = src[i];

		//for spaces
        if (c == '+')
		{
            result.push_back(' ');
        }
		//for special chars
        else if (c == '%' && i + 2 < src.size())
		{
            char h1 = src[i + 1];
            char h2 = src[i + 2];
			//turns a %xx into an actual char
            result.push_back(static_cast<char>((fromHex(h1) << 4) | fromHex(h2)));
            i += 2;
        } 
        else
		{
            result.push_back(c);
        }
    }
    return result;
}

std::map<std::string, std::string> Server::ft_parse_post_data(const std::string& body)
{
	std::map<std::string, std::string> params;
	
	if (body.empty())
		return params;
	
	// Exemple de body : "username=john&password=secret&email=john@test.com"
	//not really-> html parses special chars like spaces and @ as other forms
	std::string current_body = bodyDecode(body);
	
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
        return ft_build_post_success_response("Upload simule - aucun fichier specifie");
    }
    
    std::string filename = file_it->second;
    std::string filepath = "www/uploads/" + filename;
    std::string content = (content_it != params.end()) ? content_it->second : "contenu vide";
    
    // Save file
    std::ofstream file(filepath.c_str(), std::ios::binary);
    if (!file.is_open())
	{
		std::cout << "file : " << filepath << " failed to open" << std::endl;
		if (errno == EACCES || errno == EISDIR)
        	return ft_build_403_response(); 	
		else if (errno == ENOENT)
        	return ft_build_404_response();
		else
		{
        	return ft_build_500_response();
    	}
	}

    file.write(content.c_str(), content.size());
    file.close();
    
    // Append to file listing
    std::ofstream file1("www/files.html", std::ios::app);
    if (!file1.is_open())
	{
        std::cout << "file : " << filepath << " failed to open" << std::endl;
		if (errno == EACCES || errno == EISDIR)
        	return ft_build_403_response(); 	
		else if (errno == ENOENT)
        	return ft_build_404_response();
		else
		{
        	return ft_build_500_response();
    	}
    }

    file1 << "<li><a href=\"/uploads/" << filename << "\">" << filename << "</a></li>\n";
    file1.close();
    
    std::string success_msg = "File '" + filename + "' uploaded";
    return ft_build_post_success_response(success_msg);
}

//Gérer le formulaire de contact
std::string Server::ft_handle_contact(const std::map<std::string, std::string>& params)
{
	std::map<std::string, std::string>::const_iterator name_it = params.find("name");
	std::map<std::string, std::string>::const_iterator email_it = params.find("email");
	std::map<std::string, std::string>::const_iterator message_it = params.find("message");
	std::map<std::string, std::string>::const_iterator priority_it = params.find("priority");
	std::map<std::string, std::string>::const_iterator subject_it = params.find("subject");
	
	std::string name = (name_it != params.end()) ? name_it->second : "Anonyme";
	std::string email = (email_it != params.end()) ? email_it->second : "Non spécifié";
	std::string message = (message_it != params.end()) ? message_it->second : "message vide";
	std::string priority = (priority_it != params.end()) ? priority_it->second : "Normal";
	std::string subject = (subject_it != params.end()) ? subject_it->second : "Message vide";
	std::string timestamp = ft_get_timestamp();
	
	std::string response_msg = "Message reçu de " + name + " (" + email + "): " + message;

	//opening of the html file to add the message
	std::ofstream file("www/messages.html", std::ios::app);
    if (!file)
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

	file << "      <div class=\"message-card\">\n"
     << "        <div class=\"message-header\">\n"
     << "          <span class=\"sender\">" << name << "</span>\n"
     << "          <span class=\"email\">&lt;" << email << "&gt;</span>\n"
     << "          <span class=\"priority " << priority << "\">"
     << (priority == "urgent" ? "Urgent" : "Normal") << "</span>\n"
     << "        </div>\n"
     << "        <div class=\"subject\">" << subject << "</div>\n"
     << "        <div class=\"message-body\">" << message << "</div>\n"
     << "        <div class=\"timestamp\">Reçu le: " << timestamp << "</div>\n"
     << "      </div>\n\n";



    file.close();
    std::cout << "Message appended to messages.html\n";
	return ft_build_post_success_response(response_msg);
}
