/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MyServer.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 11:59:15 by qliso             #+#    #+#             */
/*   Updated: 2025/05/04 14:07:09 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MYSERVER_HPP
# define MYSERVER_HPP

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

    public:
        MyServer(const std::vector<std::string>& ports);
        ~MyServer(void);

        void    runServer(void);
};


#endif