/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MyClientConnection.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 14:55:32 by qliso             #+#    #+#             */
/*   Updated: 2025/05/07 20:12:03 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MYCLIENTCONNECTION_HPP
# define MYCLIENTCONNECTION_HPP

# include <string>
# include <iostream>
# include <cstdlib>
# include <unistd.h>

class   MyClientConnection
{
    private:
		int			_fd;
		std::string	_requestBuffer;
		bool		_requestComplete;

    public:
		MyClientConnection(int fd);
		~MyClientConnection(void);

		int					getFd(void) const;
		const std::string& 	getRequestBuffer(void) const;
		bool				isRequestComplete(void) const;
		void				appendToBuffer(const char* data, ssize_t len);
		void				clearBuffer(void);

		void				checkRequestCompletion(void);
};

#endif