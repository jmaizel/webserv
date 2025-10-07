/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haroldsorel <marvin@42.fr>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/28 15:29:00 by haroldsorel       #+#    #+#             */
/*   Updated: 2025/09/28 15:29:01 by haroldsorel      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <ctime>
#include <iostream>
#include <unistd.h>

class Client
{
    private:
        int         _fd;    //socket file descriptor
        time_t      _timeout;   //last activity timestamp

    public:
        std::string _buffer;
        Client(int fd);
        ~Client();
        int getFd()const;
        time_t getTimeout()const;
        void    resetTime();
        bool hasTimedOut(int timeoutSec)const;
};
