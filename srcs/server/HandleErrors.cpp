/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HandleErrors.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 14:32:06 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/12 14:32:07 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "../../includes/main.hpp"


HttpResponse Server::generate_success_response(int code, const std::string &reason, const std::string &body)
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode(code);
    res.setReason(reason);

    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}

HttpResponse Server::generate_error_response(int code, const std::string &reason, const std::string &details)
{
    HttpResponse res;

    res.setVersion("HTTP/1.1");
    res.setStatusCode(code);
    res.setReason(reason);

    std::string body = "<html><body><h1>" + std::to_string(code) + " " + reason + "</h1><p>" + details + "</p></body></html>";

    res.setHeaders("Content-Type", "text/html");
    res.setHeaders("Content-Length", std::to_string(body.size()));
    res.setHeaders("Connection", "close");
    res.setBody(body);

    return (res);
}