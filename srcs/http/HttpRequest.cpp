/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:41:00 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:41:01 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

/*  TEMPLATE

first line      GET /index.html HTTP/1.1\r\n
header          Host: localhost:8080\r\n
header          User-Agent: Mozilla/5.0 (X11; Linux x86_64)\r\n
header          Accept: text/html\r\n
header          Connection: keep-alive\r\n
\r\n
body(optional)  {"username":"harold","password":"42"}

*/

HttpRequest::HttpRequest() :
    _buffer(""),
    _query_string(""),
    _method(""),
    _target(""),
    _version(""),
    _headers(),
    _body(""),
    _flag(0)
{

}

HttpRequest::HttpRequest(const HttpRequest &copy)
{
    (void)copy;
}

HttpRequest::~HttpRequest()
{

}

const std::string                         &HttpRequest::getMethod()const
{
    return (this->_method);
}

const std::string                         &HttpRequest::getTarget()const
{
    return (this->_target);
}

const std::string                         &HttpRequest::getVersion()const
{
    return (this->_version);
}

const std::map<std::string, std::string>    &HttpRequest::getHeaders()const
{
    return (this->_headers);
}

const std::string                         &HttpRequest::getBody()const
{
    return (this->_body);
}

const std::string                         &HttpRequest::getQueryString()const
{
    return (this->_query_string);
}

const int   &HttpRequest::getFlag()const
{
    return (this->_flag);
}

std::string normalize_uri(const std::string &uri)
{
    std::vector<std::string> parts;
    std::stringstream ss(uri);
    std::string item;

    //split by '/' and process each part
    while (std::getline(ss, item, '/'))
    {
        if (item.empty() || item == ".")
        {
            //skip empty (handles //) and "."
            continue;
        }
        if (item == "..")
        {
            // go one level up if possible
            if (!parts.empty())
                parts.pop_back();
        }
        else
        {
            parts.push_back(item);
        }
    }

    //build normalized path
    std::string result = "/";
    for (size_t i = 0; i < parts.size(); i++)
    {
        result += parts[i];
        if (i + 1 < parts.size())
            result += "/";
    }
    return result;
}

void    HttpRequest::print()const
{
    std::map<std::string, std::string>::const_iterator it;

    std::cout << "=== HttpRequest ===" << std::endl;

    std::cout << this->_method << " " << this->_target << " " << this->_version << std::endl;

    for (it = this->_headers.begin(); it != this->_headers.end(); ++it)
        std::cout << it->first << ": " << it->second << std::endl;

    std::cout << this->_body << std::endl;

    std::cout << "====================" << std::endl;
}

//tokenizes based on /r/n. elements are seperated by spaces
std::vector<std::string> HttpRequest::tokenize(const std::string buffer)const
{
    std::string                 copy(buffer);
    std::string                 line;
    size_t                      pos;
    std::vector<std::string>    tokens;

    while ((pos = copy.find("\r\n")) != std::string::npos)
    {
        line = copy.substr(0, pos);
        if (!line.empty()) //if there is only "\r\n"
            tokens.push_back(line);
        tokens.push_back(" ");
        copy.erase(0, pos + 2);
    }
    //rest of the buffer
    if (!copy.empty())
        tokens.push_back(copy);
    return (tokens);
}


bool    HttpRequest::is_valid_headers()const
{
    //required fields
    if (_version.empty() || _method.empty() || _target.empty())
        throw std::runtime_error("Missing required request line");
    //target must start with /
    if (_target[0] != '/')
        throw std::runtime_error("Invalid request target");
    //HTTP version
    if (_version != "HTTP/1.1" && _version != "HTTP/1.0")
        throw std::runtime_error("HTTP Version Not Supported");

    //Content-Length check
    std::map<std::string, std::string>::const_iterator it = _headers.find("Content-Length");
    if (it != _headers.end())
    {
        try { size_t len = safe_atosize_t(it->second); (void)len;}
            catch (std::exception &e) {throw std::runtime_error("400 Invalid Content-Length");}
    }

    //transfer-Encoding rules
    if (_version == "HTTP/1.0")
    {
        if (_headers.find("Transfer-Encoding") != _headers.end())
            throw std::runtime_error("400 Transfer-Encoding not allowed in HTTP/1.0");
    }
    else if (_version == "HTTP/1.1")
    {
        it = _headers.find("Transfer-Encoding");
        if (it != _headers.end() && it->second == "chunked" && _headers.find("Content-Length") != _headers.end())
            throw std::runtime_error("400 Cannot have both Content-Length and chunked encoding");
    }
    return (true);
}

void    HttpRequest::setBody(const std::string &str)
{
    this->_body = str;
}

void    HttpRequest::parse(const std::string &buffer)
{
    std::vector<std::string>    tokens;
    std::vector<std::string>    elems;
    size_t                      pos;
    std::string                 key;
    std::string                 value;

    //check if there is a /r/n after the headers
    if (buffer.find("\r\n\r\n") == std::string::npos)
        throw std::runtime_error("Missing CRLFCRLF after headers");

    tokens = tokenize(buffer);
    //check if it is not empty
    if (tokens.size() < 2)
        throw std::runtime_error("Empty request");

    //split the first line
    elems = ft_split(tokens[0], " ");
    //check if there are 3 elements
    if (elems.size() < 3)
        throw std::runtime_error("Invalid request line");

    this->_method = elems[0];

    //the uri can have a query string
    if (elems[1].find("?") != std::string::npos)
    {
        size_t pos = elems[1].find_first_of("?");
        this->_target = normalize_uri(elems[1].substr(0, pos));
        if (this->_target.empty())
            throw std::runtime_error("Invalid URI");
        this->_query_string = elems[1].substr(pos + 1); 
    }
    else
    {
        this->_target = normalize_uri(elems[1]);
    }
    this->_version = elems[2];

    //check the headers
    size_t i = 2;
    for (; i < tokens.size() ; ++i)
    {
        if (tokens[i] == " ")
        {
            if (i < tokens.size() - 1 && tokens[i + 1] == " ")
            {
                i += 2;
                break ;
            }
            continue ;
        }
        pos = (tokens[i]).find_first_of(":");
        if (pos == std::string::npos)
            throw std::runtime_error("Malformed header line");

        key = (tokens[i]).substr(0, pos);
        value = (tokens[i]).substr(pos + 2);
        (this->_headers)[key] = value;
    }
    try {is_valid_headers();}
    catch(std::exception &e) {throw;}
}