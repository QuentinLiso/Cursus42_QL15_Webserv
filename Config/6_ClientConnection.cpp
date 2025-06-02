/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/02 13:14:52 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "6_ClientConnection.hpp"





ClientConnection::ClientConnection(int fd)
        :   _fd(fd),
            _recvBuffer(),
			_requestComplete(false)
{}

ClientConnection::~ClientConnection(void) {}

int	ClientConnection::getFd(void) const { return _fd; }
const TStr&	ClientConnection::getRequestBuffer(void) const { return _recvBuffer; }
bool	ClientConnection::isRequestComplete(void) const { return _requestComplete; }

