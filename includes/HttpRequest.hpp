/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 18:42:33 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 18:42:35 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>

/*  TEMPLATE

first line      GET /index.html HTTP/1.1\r\n
header          Host: localhost:8080\r\n
header          User-Agent: Mozilla/5.0 (X11; Linux x86_64)\r\n
header          Accept: text/html\r\n
header          Connection: keep-alive\r\n
\r\n
body(optional)  {"username":"harold","password":"42"}

*/

class HttpRequest
{
    private:
        std::string                         _buffer;
        std::string                         _method;
        std::string                         _target;
        std::string                         _version;
        std::map<std::string, std::string>  _headers;
        std::string                         _body;
        int                                 _flag;

    public:
        //constructors
        HttpRequest();
        HttpRequest(const HttpRequest &copy);

        //destructors
        ~HttpRequest();

        //method
        void    parse(const std::string &buffer);
        void    print();
};
