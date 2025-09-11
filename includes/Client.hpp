/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 18:44:02 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 18:44:03 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

class Client
{
    private:
        int                 _socket_fd;  //clients socket (created by accept())
        std::string         _request; //http requests arrive in pieces
        std::string         _response;
    public:
        Client();
        ~Client();   
};
