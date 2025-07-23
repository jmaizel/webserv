/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerFiles.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/23 11:52:15 by jmaizel           #+#    #+#             */
/*   Updated: 2025/07/23 11:58:24 by jmaizel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.hpp"
#include <sstream>

std::string Server::ft_serve_static_file(const std::string& uri)
{
	std::cout << "Trying to serve file for URI: " << uri << std::endl;
	
	// 1. Construire le chemin du fichier sur le système
	std::string file_path = "./www";
	if (uri == "/") {
		file_path += "/index.html";
	} else {
		file_path += uri;
	}
	
	// 2. Lire le contenu du fichier
	std::string file_content = ft_read_file_simple(file_path);
	
	// 3. Vérifier si le fichier existe
	if (file_content.empty()) {
		// Fichier introuvable → retourner 404
		return ft_build_404_response();
	}
	
	// 4. Déterminer le Content-Type selon l'extension
	std::string content_type = ft_get_content_type(file_path);
	
	// 5. Calculer Content-Length
	std::ostringstream oss;
	oss << file_content.length();
	
	// 6. Construire la réponse HTTP 200 OK
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: " + content_type + "\r\n";
	response += "Content-Length: " + oss.str() + "\r\n\r\n";
	response += file_content;
	
	return response;
}

std::string Server::ft_read_file_simple(const std::string& file_path)
{
	std::cout << "Attempting to read file: " << file_path << std::endl;
	
	// 1. Ouvrir le fichier en lecture
	std::ifstream file(file_path.c_str());
	if (!file.is_open()) {
		std::cout << "Error: Cannot open file " << file_path << std::endl;
		return "";  // Retourner chaîne vide si échec
	}
	
	// 2. Lire le fichier ligne par ligne
	std::string content;
	std::string line;
	
	while (std::getline(file, line)) {
		content += line + "\n";  // Reconstituer le fichier
	}
	
	// 3. Fermer le fichier et afficher le résultat
	file.close();
	std::cout << "Successfully read " << content.length() << " bytes" << std::endl;
	return content;
}

std::string Server::ft_get_content_type(const std::string& file_path)
{
	// 1. Trouver l'extension du fichier
	size_t dot_pos = file_path.find_last_of('.');
	if (dot_pos == std::string::npos) {
		return "application/octet-stream";  // Type par défaut
	}
	
	std::string extension = file_path.substr(dot_pos);
	
	// 2. Déterminer le Content-Type selon l'extension
	if (extension == ".html" || extension == ".htm") {
		return "text/html";               // Pages web
	} else if (extension == ".css") {
		return "text/css";                // Feuilles de style
	} else if (extension == ".js") {
		return "application/javascript";  // Scripts JavaScript
	} else if (extension == ".png") {
		return "image/png";               // Images PNG
	} else if (extension == ".jpg" || extension == ".jpeg") {
		return "image/jpeg";              // Images JPEG
	} else if (extension == ".gif") {
		return "image/gif";               // Images GIF
	} else if (extension == ".ico") {
		return "image/x-icon";            // Favicons
	} else if (extension == ".txt") {
		return "text/plain";              // Fichiers texte
	} else if (extension == ".json") {
		return "application/json";        // Données JSON
	} else if (extension == ".pdf") {
		return "application/pdf";         // Documents PDF
	} else if (extension == ".zip") {
		return "application/zip";         // Archives
	} else {
		return "application/octet-stream"; // Type générique pour le reste
	}
}
