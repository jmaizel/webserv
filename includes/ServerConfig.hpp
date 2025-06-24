/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/12 11:35:25 by mwattier          #+#    #+#             */
/*   Updated: 2025/06/24 11:39:41 by jmaizel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

struct LocationConfig {
    std::string path;
    std::string root;
    std::string index;
    std::vector<std::string> allowed_methods;
    int client_max_body_size;
};

struct ServerConfig {
    int listen;
    std::string server_name;
    std::string root;
    std::string index;
    std::vector<std::string> allowed_methods;
    std::vector<LocationConfig> locations;
    std::map<int, std::string> error_pages;
};

ServerConfig parseConfigFile(const std::string& filename);

#endif