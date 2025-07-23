/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestLine.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 11:51:29 by jmaizel           #+#    #+#             */
/*   Updated: 2025/07/23 11:51:30 by jmaizel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	std::string method, uri_with_query, version;
	
	// Extraire les 3 parties
	if (!(iss >> method >> uri_with_query >> version))
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
	
	if (uri_with_query.empty() || uri_with_query[0] != '/')
	{
		request.error_message = "Invalid URI: " + uri_with_query;
		return false;
	}
	
	// Séparer l'URI de la query string
	size_t query_pos = uri_with_query.find('?');
	if (query_pos != std::string::npos)
	{
		// Il y a une query string
		request.uri = uri_with_query.substr(0, query_pos);
		request.query_string = uri_with_query.substr(query_pos + 1);
	}
	else
	{
		// Pas de query string
		request.uri = uri_with_query;
		request.query_string = "";
	}
	
	// Stocker les autres valeurs
	request.method = method;
	request.version = version;
	
	return true;
}