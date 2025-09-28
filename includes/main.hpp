/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 17:48:30 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 17:48:31 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "ServerMonitor.hpp"
#include "Config.hpp"
#include "Client.hpp"
#include <stdexcept>
#include <limits>
#include <sys/types.h>
#include <dirent.h>

#define HEADER_TIMEOUT 60
#define BODY_TIMEOUT 60
#define MAX_CLIENTS 1000
#define TIMEOUT_SEC 30

std::string                 strtrim(std::string &str);
std::string                 brackettrim(std::string &line);
std::string                 spacetrim(std::string &line);
bool                        is_closed(std::string str);
std::vector<std::string>    ft_split(const std::string &line, const std::string &separators);
int                         get_closing_bracket_index(std::string &str);
std::string                 commenttrim(std::string &str);
size_t                      safe_atosize_t(const std::string &str);
std::string                 trimmer(std::string &str);

template <typename T>
std::string to_string98(const T &value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}