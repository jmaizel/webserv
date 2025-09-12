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
#include <stdexcept>
#include <limits>
#include <sys/types.h>
#include <dirent.h>

std::string                 strtrim(std::string &str);
std::string                 brackettrim(std::string &line);
std::string                 spacetrim(std::string &line);
bool                        is_closed(std::string str);
std::vector<std::string>    ft_split(const std::string &line, const std::string &separators);
int                         get_closing_bracket_index(std::string &str);
std::string                 commenttrim(std::string &str);
size_t                      safe_atosize_t(const std::string &str);
std::string                 trimmer(std::string &str);