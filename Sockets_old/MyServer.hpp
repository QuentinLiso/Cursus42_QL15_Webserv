/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MyServer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 11:59:15 by qliso             #+#    #+#             */
/*   Updated: 2025/05/13 10:50:43 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MYSERVER_HPP
# define MYSERV_HPP

# include <vector>
# include <map>
# include <string>
# include <poll.h>
# include "MySocket.hpp"
# include "MyClientConnection.hpp"

class   MyServer
{
    private:
        std::vector<std::string>            _ports;
        MySocket                            _listeningSocket;
        std::vector<struct pollfd>          _fds;
        std::map<int, MyClientConnection>   _clients;

        void    _init(void);
        void    _mainLoop(void);
        void    _handleNewConnection(void);
        void    _handleClientActivity(size_t index);
        void    _handleNoByteRead(int fd, size_t index, const std::string &msg);
        int    _handleBytesRead(int fd, char buffer[], ssize_t bytesRead);

    public:
        MyServer(const std::vector<std::string>& ports);
        ~MyServer(void);

        void    runServer(void);
};


#endif