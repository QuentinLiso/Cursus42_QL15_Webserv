/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:03 by qliso             #+#    #+#             */
/*   Updated: 2025/06/02 23:28:59 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_CONNECTION_HPP
# define CLIENT_CONNECTION_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"

class ClientConnection
{
private:
    int     _fd;
	size_t	_maxBytes;
    TStr    _recvBuffer;
    bool    _requestComplete;
    
public:
    ClientConnection(int fd);
    virtual ~ClientConnection(void);

    int     getFd(void) const;
    const TStr&	getRequestBuffer(void) const;
	bool	isRequestComplete(void) const;

	int		readFromFd(void);
};



#endif