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
#include <vector>

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

        //private methods
        bool                        is_valid_request()const;
        std::vector<std::string>    tokenize(const std::string buffer)const;

    public:
        //constructors
        HttpRequest();
        HttpRequest(const HttpRequest &copy);

        //destructors
        ~HttpRequest();

        //getters
        const std::string                           &getMethod()const;
        const std::string                           &getTarget()const;
        const std::string                           &getVersion()const;
        const std::map<std::string, std::string>    &getHeaders()const;
        const std::string                           &getBody()const;
        const int                                   &getFlag()const;


        //public methods
        void                        parse(const std::string &buffer);
        void                        print()const;
};
