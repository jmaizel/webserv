/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hsorel <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 17:27:36 by hsorel            #+#    #+#             */
/*   Updated: 2025/09/08 17:27:38 by hsorel           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/main.hpp"

int main (int argc, char **argv)
{

    signal(SIGINT, ServerMonitor::handle_sigint);
    signal(SIGTERM, ServerMonitor::handle_sigint);
    
    std::string config = "config/default.conf";

    if (argc > 2)
    {
        std::cerr << "Too many arguments: Enter only one config file" << std::endl;
        return (1);
    }

    if (argc == 2 && argv[1])
        config = argv[1];

    try
    {
        ServerMonitor webserv(config);
        std::cout << "\033[31mPARSING SERVERS...\033[0m" << std::endl;
        std::cout << std::endl;
        webserv.parse();
        std::cout << std::endl;
        std::cout << "\033[31mPRINTING AVAILABLE SERVERS...\033[0m" << std::endl;
        std::cout << std::endl;
        webserv.print();
        std::cout << std::endl;
        std::cout << "\033[31mLAUCHING SERVERS...\033[0m" << std::endl;
        std::cout << std::endl;
        webserv.init_servers();
        std::cout << std::endl;
        std::cout << "\033[31mAWAITING CLIENT CONNECTIONS..\033[0m." << std::endl;
        std::cout << std::endl;
        webserv.run();
        std::cout << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return (EXIT_SUCCESS);
}
