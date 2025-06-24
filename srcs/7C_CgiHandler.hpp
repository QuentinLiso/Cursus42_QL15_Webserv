/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7C_CgiHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/19 16:19:43 by qliso             #+#    #+#             */
/*   Updated: 2025/06/24 11:54:57 by qliso            ###   ########.fr       */
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

class 	Server;

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

		CgiHandler(Server& server);
		~CgiHandler(void);


	private:
		Server&		_server;
		CgiState	_cgiState;
		int			_cgiStatusCode;
		bool		_outOnly;

		pid_t		_pid;
		int			_inputPipeFd[2];
		int			_outputPipeFd[2];

		// Cgi input
		int			_requestBodyInputFd;	
		TStr		_cgiWriteInputBuffer;
		size_t		_cgiWriteOffset;
		bool		_cgiWriteToInputComplete;
		size_t		_actualBytesWrittenToCgiInput;

		// Cgi output
		TStr		_cgiOutputCompleteFilepath;
		static uint	_cgiOutputCompleteFdTmpCount;
		int			_cgiOutputCompleteFd;
		TStr		_cgiReadOutputBuffer;
		size_t		_cgiReadOffset;
		bool		_cgiReadFromOutputComplete;
		size_t		_actualBytesReadFromCgiOutput;

		bool					_cgiHeadersEnd;
		size_t					_cgiHeadersEndIndex;
		size_t					_index;
		std::map<TStr, TStr>	_cgiOutputHeaders;


		CgiState	setupCgiOutput(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution);
		CgiState	setupCgiInputOutput(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution);
		void		buildExecveArgsEnv(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution, std::vector<char*>& argv, TStrVect& tmpEnvp, std::vector<char*>& envp);
		CgiState	openOutputCompleteFile(void);
		bool		findHeadersEnd(void);
		void		parsingOutputHeaders(void);

		CgiState		errorSetup(int cgiStatusCode, const TStr& msg, CgiState cgiState);
		CgiState		errorRunning(int cgiStatusCode, const TStr& msg, CgiState cgiState);

	public:
		CgiState	setupCgi(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution);
		bool		writeToCgiInputPipe(void);
		bool		readFromCgiOutputPipe(void);
		bool		flushBuffer(void);
		
		
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
		bool		isCgiWriteFromInputComplete(void) const;
		void		setOutOnly(bool val);

		size_t		getActualBytesWrittenToCgiInput(void) const;
		size_t		getActualBytesReadFromCgiOutput(void) const;
};

#endif