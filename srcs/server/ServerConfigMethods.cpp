#include "Server.hpp"

// Trouve la location qui correspond le mieux à l'URI
const LocationConfig* Server::ft_find_location(const std::string& uri)
{
	const LocationConfig* best_match = NULL;
	size_t best_match_length = 0;
	
	// Parcourir toutes les locations configurées
	size_t i = 0;
	while (i < _config.locations.size())
	{
		const LocationConfig& location = _config.locations[i];
		
		// Vérifier si l'URI commence par le path de cette location
		if (uri.find(location.path) == 0)
		{
			// Prendre la location avec le path le plus long (plus précise)
			if (location.path.length() > best_match_length)
			{
				best_match = &location;
				best_match_length = location.path.length();
			}
		}
		i++;
	}
	
	return best_match;
}

// Vérifie si une méthode HTTP est autorisée
bool Server::ft_is_method_allowed(const std::string& method, const LocationConfig* location)
{
	const std::vector<std::string>* allowed_methods;
	
	// Utiliser les méthodes de la location si elle existe, sinon celles du serveur
	if (location && !location->allowed_methods.empty())
		allowed_methods = &location->allowed_methods;
	else
		allowed_methods = &_config.allowed_methods;
	
	// Si aucune restriction définie, autoriser GET par défaut
	if (allowed_methods->empty())
		return (method == "GET");
	
	// Vérifier si la méthode est dans la liste
	size_t i = 0;
	while (i < allowed_methods->size())
	{
		if ((*allowed_methods)[i] == method)
			return true;
		i++;
	}
	
	return false;
}

// Construit le chemin complet du fichier selon la config
std::string Server::ft_get_file_path(const std::string& uri, const LocationConfig* location)
{
	std::string root;
	std::string index;
	
	// Utiliser root et index de la location si elle existe
	if (location)
	{
		root = location->root.empty() ? _config.root : location->root;
		index = location->index.empty() ? _config.index : location->index;
		
		std::cout << "Using location config - root: " << root << ", index: " << index << std::endl;
	}
	else
	{
		root = _config.root;
		index = _config.index;
		
		std::cout << "Using server config - root: " << root << ", index: " << index << std::endl;
	}
	
	// Construire le chemin
	std::string file_path = root;
	
	if (uri == "/")
	{
		// Page d'accueil
		if (file_path[file_path.length() - 1] != '/')
			file_path += "/";
		file_path += index;
	}
	else
	{
		// Fichier spécifique
		if (location && !location->path.empty() && uri.find(location->path) == 0)
		{
			// Enlever le path de la location de l'URI
			std::string relative_path = uri.substr(location->path.length());
			if (!relative_path.empty() && relative_path[0] == '/')
				relative_path = relative_path.substr(1);
			
			if (file_path[file_path.length() - 1] != '/' && !relative_path.empty())
				file_path += "/";
			file_path += relative_path;
		}
		else
		{
			// Pas de location spécifique, utiliser l'URI tel quel
			file_path += uri;
		}
	}
	
	return file_path;
}