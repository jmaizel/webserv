/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 18:42:22 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 18:42:24 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "HttpRequest.hpp"

/*  TEMPLATE

first line  HTTP/1.1 200 OK\r\n
header      Content-Type: text/html\r\n
header      Content-Length: 32\r\n
header      Connection: close\r\n
\r\n
body        <html><h1>Hello World</h1></html>

*/

class HttpResponse
{
    private:
        std::string                         version;
        int                                 statusCode;
        std::string                         reason;
        std::map<std::string, std::string>  headers;
        std::string                         body;


    public:
        //constructors
        HttpResponse();
        HttpResponse(const HttpRequest &req);

        //destructors
        ~HttpResponse();

        //methods
        void        parse(const HttpRequest &req);
        std::string toStr();
};
