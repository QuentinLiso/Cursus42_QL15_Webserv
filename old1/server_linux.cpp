/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_linux.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 14:07:12 by qliso             #+#    #+#             */
/*   Updated: 2025/04/28 17:12:23 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_tcpServer_linux.hpp"
#include <iostream>

int main()
{
    using namespace http;

    TcpServer   server = TcpServer("127.0.0.1", 8080);
    server.startListen();
    return (0);
}