/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   8_HttpResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 09:23:08 by qliso             #+#    #+#             */
/*   Updated: 2025/06/04 12:59:32 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "8_HttpResponse.hpp"

const char*	const* HttpResponse::httpStatusCodes = HttpResponse::initHttpStatusCodes();

const char**	HttpResponse::initHttpStatusCodes(void)
{
	static const char*	array[512] = { NULL };

	array[100] = "Continue";
	array[101] = "Switching Protocols";
	array[102] = "Processing";
	array[103] = "Early Hints";
	array[200] = "OK";
	array[201] = "Created";
	array[202] = "Accepted";
	array[203] = "Non-Authoritative Information";
	array[204] = "No Content";
	array[205] = "Reset Content";
	array[206] = "Partial Content";
	array[207] = "Multi-Status";
	array[208] = "Already Reported";
	array[226] = "IM Used";
	array[300] = "Multiple Choices";
	array[301] = "Moved Permanently";
	array[302] = "Found";
	array[303] = "See Other";
	array[304] = "Not Modified";
	array[307] = "Temporary Redirect";
	array[308] = "Permanent Redirect";
	array[400] = "Bad Request";
	array[401] = "Unauthorized";
	array[402] = "Payment Required";
	array[403] = "Forbidden";
	array[404] = "Not Found";
	array[405] = "Method Not Allowed";
	array[406] = "Not Acceptable";
	array[407] = "Proxy Authentication Required";
	array[408] = "Request Timeout";
	array[409] = "Conflict";
	array[410] = "Gone";
	array[411] = "Length Required";
	array[412] = "Precondition Failed";
	array[413] = "Content Too Large";
	array[414] = "URI Too Long";
	array[415] = "Unsupported Media Type";
	array[416] = "Range Not Satisfiable";
	array[417] = "Expectation Failed";
	array[418] = "I'm a teapot";
	array[421] = "Misdirected Request";
	array[422] = "Unprocessable Content";
	array[423] = "Locked";
	array[424] = "Failed Dependency";
	array[425] = "Too Early";
	array[426] = "Upgrade Required";
	array[428] = "Precondition Required";
	array[429] = "Too Many Requests";
	array[431] = "Request Header Fields Too Large";
	array[451] = "Unavailable For Legal Reasons";
	array[500] = "Internal Server Error";
	array[501] = "Not Implemented";
	array[502] = "Bad Gateway";
	array[503] = "Service Unavailable";
	array[504] = "Gateway Timeout";
	array[505] = "HTTP Version Not Supported";
	array[506] = "Variant Also Negotiates";
	array[507] = "Insufficient Storage";
	array[508] = "Loop Detected";
	array[510] = "Not Extended";
	array[511] = "Network Authentication Required";
	
	return (array);
}

const char*	HttpResponse::getStatusCodeReason(unsigned short httpStatusCode)
{
	if (httpStatusCode < 100 || httpStatusCode > 511 || HttpResponse::httpStatusCodes[httpStatusCode] == NULL)
		return (HttpResponse::httpStatusCodes[500]);
	return (HttpResponse::httpStatusCodes[httpStatusCode]);
}

HttpResponse::HttpResponse(void) {}
HttpResponse::~HttpResponse(void) {}

void	HttpResponse::print(void) const
{
	for (size_t i = 0; i < 600; i++)
	{
		std::cout << "Error code : " << i << " -> " << HttpResponse::getStatusCodeReason(i) << std::endl;
	}
}