/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7C_CgiHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/19 16:19:43 by qliso             #+#    #+#             */
/*   Updated: 2025/06/22 00:21:21 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"
#include "5_ListeningSocket.hpp"
#include "7_HttpRequest.hpp"
#include "7A_HttpRequestData.hpp"
#include "7B_HttpRequestResolution.hpp"
#include "8_HttpResponse.hpp"

class	CgiHandler
{
	public:
		enum	CgiState
		{
			CGI_SETUP,
			CGI_SETUP_VALID,
			CGI_SETUP_ERROR,
			CGI_RUNNING,
			CGI_RUNNING_ERROR,
			CGI_WRITING_TO_INPUT_DONE,
			CGI_READING_FROM_OUTPUT_DONE,
		};

		enum	CgiParsingState
		{
			PARSING_HEADERS_NEED_DATA,	// OK
			PARSING_HEADERS_END_FOUND,	// OK
			PARSING_REQUEST_LINE_DONE,	// OK
			PARSING_HEADERS_PROCESSING, // OK
			PARSING_HEADERS_DONE,		// OK
			PARSING_HEADERS_INVALID,	// OK
		};

		CgiHandler(void);
		~CgiHandler(void);


	private:
		CgiState	_cgiState;
		int			_cgiStatusCode;
		bool		_outOnly;

		pid_t		_pid;
		int			_inputPipeFd[2];
		int			_outputPipeFd[2];

		int			_requestBodyInputFd;	

		CgiParsingState	_cgiParsingState;
		TStr		_cgiOutputBuffer;
		size_t		_index;
		size_t		_cgiHeadersEndIndex;
		size_t		_cgiHeadersSize;
		size_t		_cgiHeadersCount;
		ushort		_cgiOutputHeaderStatus;
		int			_cgiOutputContentLength;
		std::map<TStr, TStr>	_cgiOutputHeaders;

		bool		_cgiOutputHeadersParsingComplete;
		int			_cgiOutputCompleteFd;
		static uint	_cgiOutputCompleteFdTmpCount;
		TStr		_cgiOutputCompleteFilepath;
		ssize_t		_totalCgiOutputSize;
		bool		_cgiReadFromOutputComplete;

		size_t		_actualBytesWrittenToCgiInput;
		size_t		_actualBytesReadFromCgiOutput;

		CgiState	setupCgiOutput(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution);
		CgiState	setupCgiInputOutput(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution);
		void		buildExecveArgsEnv(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution, std::vector<char*>& argv, TStrVect& tmpEnvp, std::vector<char*>& envp);
		CgiState	openOutputCompleteFile(void);
		CgiState	handleCgiOutputHeadersIncomplete(char readBuffer[], size_t bytesRead, size_t readBufferSize);
		
		CgiParsingState	findHeadersEnd(char readBuffer[], size_t bytesRead, size_t readBufferSize);
		CgiParsingState	parseHeaders(void);
		CgiParsingState	parseHeaderLine(const TStr& headerLine);

		CgiState	errorSetup(int cgiStatusCode, const TStr& msg, CgiState cgiState);
		CgiState	errorRunning(int cgiStatusCode, const TStr& msg, CgiState cgiState);
		CgiParsingState	errorParsing(const TStr& msg, CgiParsingState cgiParsingState);

	public:
		CgiState	setupCgi(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution);
		CgiState	writeToCgiInput(void);
		CgiState	readFromCgiOutput(void);
		
		CgiState	getCgiState(void) const;
		int			getCgiStatusCode(void) const;
		bool		isOutOnly(void) const;
		pid_t		getCgiPid(void) const;
		int			getInputPipeWrite(void) const;
		int			getOutputPipeRead(void) const;

		int			getRequestBodyInputFd(void) const;
		int			getCgiCompleteOutputFd(void) const;
		const TStr&	getCgiCompleteOutputFilename(void) const;
		int			getTotalCgiOutputSize(void) const;
		const std::map<TStr, TStr>&	getCgiOutputHeaders(void) const;
		bool		isCgiReadFromOutputComplete(void) const;
		void		setOutOnly(bool val);

		size_t		getActualBytesWrittenToCgiInput(void) const;
		size_t		getActualBytesReadFromCgiOutput(void) const;
};

#endif