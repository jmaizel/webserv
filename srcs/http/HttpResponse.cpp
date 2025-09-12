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

HttpResponse::HttpResponse() : _version(""), _headers(), _statusCode(0), _reason(""), _body("")
{

}

HttpResponse::HttpResponse(const HttpRequest &copy)
{

}

HttpResponse::~HttpResponse()
{

}



void    HttpResponse::setVersion(const std::string &version)
{
    this->_version = version;
}

void    HttpResponse::setStatusCode(int code)
{
    this->_statusCode = code;
}
void    HttpResponse::setReason(const std::string &reason)
{
    this->_reason = reason;
}

void    HttpResponse::setBody(const std::string &body)
{
    this->_body = body;
}

void    HttpResponse::setHeaders(const std::string &first, const std::string &second)
{
    this->_headers[first] = second;
}

void HttpResponse::print()
{
    std::cout << "=== HttpResponse ===" << std::endl;

    std::cout << this->_version << " " << this->_statusCode << " " << this->_reason << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << "\r\n";
    }
    std::cout << "\r\n";

    std::cout << this->_body << std::endl;

    std::cout << "====================" << std::endl;
}


std::string HttpResponse::toStr()
{
    std::map<std::string, std::string>::iterator    it;
    std::ostringstream                              buffer;

    //first line
    buffer << this->_version << " " << this->_statusCode << " " << this->_reason << "\r\n";
    //headers
    for (it = this->_headers.begin(); it != this->_headers.end() ; ++it)
        buffer << it->first << ": " << it->second << "\r\n";
    //'\r\n' to finish off the headers
    buffer << "\r\n";
    //body
    buffer << this->_body;
    return (buffer.str());
}