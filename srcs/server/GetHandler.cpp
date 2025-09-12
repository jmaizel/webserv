/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:41:30 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 19:41:31 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/main.hpp"

HttpResponse    Server::generate_get_response(HttpRequest &req)
{
    HttpResponse res;

    return (res);

}

HttpResponse Server::generate_invalid_request_response()
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode("400");
    res.setReason("Bad Request");

    std::string body = "<html><body><h1>400 Bad Request</h1></body></html>";

    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}

HttpResponse Server::generate_method_not_implemented_response()
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode("501");
    res.setReason("Not Implemented");

    std::string body = "<html><body><h1>501 Not Implemented</h1></body></html>";
    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}
