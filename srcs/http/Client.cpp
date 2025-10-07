/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haroldsorel <marvin@42.fr>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/28 15:28:13 by haroldsorel       #+#    #+#             */
/*   Updated: 2025/09/28 15:28:15 by haroldsorel      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

Client::Client(int fd) : _fd(fd), _timeout(std::time(NULL)), _buffer("")
{
}

Client::~Client()
{
    if (_fd >= 0)
        close(_fd);
}

int Client::getFd()const
{
    return _fd;
}

time_t Client::getTimeout()const
{
    return _timeout;
}

void    Client::resetTime()
{
    _timeout = std::time(NULL);
}
bool Client::hasTimedOut(int timeoutSec) const
{
    return (std::time(NULL) - _timeout) > timeoutSec;
}
