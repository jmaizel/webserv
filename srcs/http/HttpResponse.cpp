/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:40:51 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:40:52 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

/*  TEMPLATE

first line  HTTP/1.1 200 OK\r\n
header      Content-Type: text/html\r\n
header      Content-Length: 32\r\n
header      Connection: close\r\n
\r\n
body        <html><h1>Hello World</h1></html>

*/

HttpResponse::HttpResponse()
{

}

HttpResponse::HttpResponse(const HttpRequest &copy)
{

}

HttpResponse::~HttpResponse()
{

}

void    HttpResponse::parse(const HttpRequest &req)
{
    
}

std::string HttpResponse::toStr()
{
    return (" ");
}