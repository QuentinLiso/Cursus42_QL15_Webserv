/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MyClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 14:56:30 by qliso             #+#    #+#             */
/*   Updated: 2025/05/08 09:39:25 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MyClientConnection.hpp"

MyClientConnection::MyClientConnection(int fd) : 
	_fd(fd), 
	_requestBuffer(""), 
	_requestComplete(false)
{

}

MyClientConnection::~MyClientConnection(void)
{
	if (_fd < 0)
		close (_fd);
}

int	MyClientConnection::getFd(void) const
{
	return (_fd);
}

const std::string&	MyClientConnection::getRequestBuffer(void) const
{
	return (_requestBuffer);
}

bool	MyClientConnection::isRequestComplete(void) const
{
	return (_requestComplete);
}

void	MyClientConnection::appendToBuffer(const char* data, ssize_t len)
{
	_requestBuffer.append(data, len);
}

void	MyClientConnection::clearBuffer(void)
{
	_requestBuffer.clear();
	_requestComplete = false;
}

void	MyClientConnection::checkRequestCompletion(void)
{
	if (_requestBuffer.find("\r\n\r\n") != std::string::npos || _requestBuffer.size() > 4096)
		_requestComplete = true;
}