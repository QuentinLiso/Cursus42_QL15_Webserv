/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7C_CgiHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/19 16:19:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/22 00:21:00 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "7C_CgiHandler.hpp"

CgiHandler::CgiHandler(void) 
		:	_cgiState(CGI_SETUP),
			_cgiStatusCode(100),
			_outOnly(true),
			_pid(-1),
			_requestBodyInputFd(-1),
			_cgiParsingState(PARSING_HEADERS_NEED_DATA),
			_index(0),
			_cgiHeadersEndIndex(0),
			_cgiHeadersSize(0),
			_cgiHeadersCount(0),
			_cgiOutputHeaderStatus(200),
			_cgiOutputContentLength(-1),
			_cgiOutputCompleteFd(-1),
			_totalCgiOutputSize(0),
			_cgiReadFromOutputComplete(false),
			_actualBytesWrittenToCgiInput(0),
			_actualBytesReadFromCgiOutput(0)
{
	_inputPipeFd[0] = -1;
	_inputPipeFd[1] = -1;
	_outputPipeFd[0] = -1;
	_outputPipeFd[1] = -1;
}

CgiHandler::~CgiHandler(void) {}

// Static
uint	CgiHandler::_cgiOutputCompleteFdTmpCount = 0;

// Functions
CgiHandler::CgiState	CgiHandler::setupCgiOutput(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution)
{
	if (pipe(_outputPipeFd) == -1)
		return (errorSetup(500, "Pipe failed", CGI_SETUP_ERROR));

	_pid = fork();
	if (_pid < 0)
		return (errorSetup(500, "Fork failed", CGI_SETUP_ERROR));

	else if (_pid == 0)
	{
		close(_outputPipeFd[0]);
		int retDup = dup2(_outputPipeFd[1], STDOUT_FILENO);
		close(_outputPipeFd[1]);
		if (retDup == -1)
			exit(500);
		TStrVect	tmpEnvp;
		std::vector<char*>	argv, envp;
		buildExecveArgsEnv(httpRequest, httpResolution, argv, tmpEnvp, envp);

		for(size_t i = 0; i < envp.size(); i++)
			std::cout << envp[i] << std::endl;

		if (chdir(httpResolution.getParentDirectory().c_str()) == -1)
			exit(500);
		execve(argv[0], argv.data(), envp.data());
		Console::log(Console::ERROR, strerror(errno));
		exit(500);
		return (CGI_SETUP_ERROR);
	}
	close(_outputPipeFd[1]);
	fcntl(_outputPipeFd[0], F_SETFL, O_NONBLOCK);
	return (openOutputCompleteFile());
}

CgiHandler::CgiState	CgiHandler::setupCgiInputOutput(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution)
{
	if (pipe(_inputPipeFd) == -1)
		return (errorSetup(500, "Pipe failed", CGI_SETUP_ERROR));

	if (pipe(_outputPipeFd) == -1)
		return (errorSetup(500, "Pipe failed", CGI_SETUP_ERROR));

	_pid = fork();
	if (_pid < 0)
		return (errorSetup(500, "Fork failed", CGI_SETUP_ERROR));

	else if (_pid == 0)
	{
		close(_inputPipeFd[1]); close(_outputPipeFd[0]);
		int retDupIn = dup2(_inputPipeFd[0], STDIN_FILENO);
		int retDupOut = dup2(_outputPipeFd[1], STDOUT_FILENO);
		close(_inputPipeFd[0]); close(_outputPipeFd[1]);

		if (retDupIn == -1 || retDupOut == -1)
			exit(500);
		TStrVect	tmpEnvp;
		std::vector<char*>	argv, envp;
		buildExecveArgsEnv(httpRequest, httpResolution, argv, tmpEnvp, envp);
		if (chdir(httpResolution.getParentDirectory().c_str()) == -1)
			exit(500);
		execve(argv[0], argv.data(), envp.data());
		Console::log(Console::ERROR, strerror(errno));
		exit(500);
		return (CGI_SETUP_ERROR);
	}
	close (_inputPipeFd[0]); close(_outputPipeFd[1]);
	fcntl(_inputPipeFd[1], F_SETFL, O_NONBLOCK);
	fcntl(_outputPipeFd[0], F_SETFL, O_NONBLOCK);
	
	_requestBodyInputFd = open(httpRequest.getRequestBodyParsingFilepath().c_str(), O_RDONLY);
	// unlink(httpRequest.getRequestBodyParsingFilepath().c_str());
	if (_requestBodyInputFd < 0)
		return (errorSetup(502, "Opening the request body file for Cgi input failed", CGI_SETUP_ERROR));
	return (openOutputCompleteFile());
}


void					CgiHandler::buildExecveArgsEnv(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution, std::vector<char*>& argv, TStrVect& tmpEnvp, std::vector<char*>& envp)
{
	argv.reserve(3);
	argv.push_back(const_cast<char*>(httpResolution.getLocationConfig()->getCgiPass().getExecPath().c_str()));
	argv.push_back(const_cast<char*>(httpResolution.getResolvedPath().c_str()));
	argv.push_back(NULL);

	// tmpEnvp.push_back("SCRIPT_FILENAME=" + httpResolution.getResolvedPath());
	// tmpEnvp.push_back("SCRIPT_NAME=" + httpRequest.getHttpRequestData().getUriPath());
	tmpEnvp.push_back("PATH_INFO=" + httpRequest.getHttpRequestData().getUriPath());
	// tmpEnvp.push_back("PATH_INFO=" + httpResolution.getResolvedPath());
	// tmpEnvp.push_back("PATH_INFO=YoupiBanane/youpi.bla");


	tmpEnvp.push_back("QUERY_STRING=" + httpRequest.getHttpRequestData().getUriQuery());
	tmpEnvp.push_back("REQUEST_METHOD=" + HttpMethods::toString(httpRequest.getHttpRequestData().getMethod()));
	tmpEnvp.push_back("CONTENT_LENGTH=" + convToStr(httpRequest.getRequestBodySize()));
	tmpEnvp.push_back("GATEWAY_INTERFACE=CGI/1.1");
	tmpEnvp.push_back("SERVER_PROTOCOL=HTTP/1.1");
	
	envp.reserve(tmpEnvp.size() + 1);
	for (size_t i = 0; i < tmpEnvp.size(); i++)
		envp.push_back(const_cast<char*>(tmpEnvp[i].c_str()));
	envp.push_back(NULL);
}

CgiHandler::CgiState	CgiHandler::openOutputCompleteFile(void)
{
	std::ostringstream	tmpFilename;
	tmpFilename << "/home/qliso/Documents/Webserv_github/html/tmp/tmp_file_cgi_" << CgiHandler::_cgiOutputCompleteFdTmpCount++ << ".txt";
	_cgiOutputCompleteFilepath = tmpFilename.str();
	_cgiOutputCompleteFd = open(_cgiOutputCompleteFilepath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (_cgiOutputCompleteFd < 0)
	{
		Console::log(Console::ERROR, strerror(errno));
		return (errorSetup(502, "Opening the tmp output file failed", CGI_SETUP_ERROR));
	}
	// unlink(_cgiOutputCompleteFilepath.c_str());
	return (CGI_SETUP_VALID);
}

CgiHandler::CgiState	CgiHandler::errorSetup(int cgiStatusCode, const TStr& msg, CgiHandler::CgiState cgiState)
{
	_cgiState = cgiState;
	_cgiStatusCode = cgiStatusCode;
	
	if (_requestBodyInputFd > 0)	close(_requestBodyInputFd);
	if (_inputPipeFd[1] > 0)		close(_inputPipeFd[1]);
	if (_outputPipeFd[0] > 0) 		close(_outputPipeFd[0]);
	if (_cgiOutputCompleteFd > 0)	close(_cgiOutputCompleteFd);

	Console::log(Console::DEBUG, msg);
	return (cgiState);
}

CgiHandler::CgiState	CgiHandler::errorRunning(int cgiStatusCode, const TStr& msg, CgiHandler::CgiState cgiState)
{
	_cgiState = cgiState;
	_cgiStatusCode = cgiStatusCode;
	
	if (_requestBodyInputFd > 0)	close(_requestBodyInputFd);
	if (_cgiOutputCompleteFd > 0)	close(_cgiOutputCompleteFd);

	Console::log(Console::DEBUG, msg);
	return (cgiState);
}

CgiHandler::CgiParsingState	CgiHandler::errorParsing(const TStr& msg, CgiHandler::CgiParsingState cgiParsingState)
{
	Console::log(Console::DEBUG, msg);
	return (cgiParsingState);
}

CgiHandler::CgiState	CgiHandler::setupCgi(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution)
{
	return (_outOnly ? setupCgiOutput(httpRequest, httpResolution) : setupCgiInputOutput(httpRequest, httpResolution));
}


CgiHandler::CgiState	CgiHandler::writeToCgiInput(void)
{
	Console::log(Console::DEBUG, "Hello from write to CGI input");
	char	buffer[8192];

	ssize_t bytesRead = read(_requestBodyInputFd, buffer, sizeof(buffer));
	if (bytesRead < 0)
		return (errorRunning(502, "Failed to read from request body input into buffer", CGI_RUNNING_ERROR));

	else if (bytesRead == 0)
	{
		close(_requestBodyInputFd);
		return (CGI_WRITING_TO_INPUT_DONE);
	}

	ssize_t	totalBytesWritten = 0;
	while (totalBytesWritten < bytesRead)
	{
		ssize_t bytesWritten = write(_inputPipeFd[1], buffer + totalBytesWritten, bytesRead - totalBytesWritten);
		if (bytesWritten < 0)
		{
			close(_requestBodyInputFd);
			return (errorRunning(502, "Failed to write from buffer into cgi input", CGI_RUNNING_ERROR));
		}

		totalBytesWritten += bytesWritten;
		_actualBytesWrittenToCgiInput += totalBytesWritten;
	}
	if (static_cast<size_t>(bytesRead) < sizeof(buffer))
	{
		close(_requestBodyInputFd);
		return (CGI_WRITING_TO_INPUT_DONE);
	}

	return (CGI_RUNNING);
}

CgiHandler::CgiState	CgiHandler::readFromCgiOutput(void)
{
	Console::log(Console::DEBUG, "Hello from read from Cgi Output");
	char	buffer[8192];

	ssize_t bytesRead = read(_outputPipeFd[0], buffer, sizeof(buffer));
	if (bytesRead < 0)
		return (errorRunning(502, "Failed to read from cgi output into buffer", CGI_RUNNING_ERROR));

	else if (bytesRead == 0)
	{
		close(_cgiOutputCompleteFd);
		_cgiReadFromOutputComplete = true;
		if (!_cgiOutputHeadersParsingComplete)
			return (errorRunning(502, "EOF reached without end of headers", CGI_RUNNING_ERROR));
		return (CGI_READING_FROM_OUTPUT_DONE);
	}
	_actualBytesReadFromCgiOutput += bytesRead;
	if (!_cgiOutputHeadersParsingComplete && !_outOnly)
		return (handleCgiOutputHeadersIncomplete(buffer, static_cast<size_t>(bytesRead), sizeof(buffer)));

	ssize_t	totalBytesWritten = 0;
	while (totalBytesWritten < bytesRead)
	{
		ssize_t bytesWritten = write(_cgiOutputCompleteFd, buffer + totalBytesWritten, bytesRead - totalBytesWritten);
		if (bytesWritten < 0)
			return (errorRunning(502, "Failed to write from buffer into cgi complete output tmp file", CGI_RUNNING_ERROR));

		totalBytesWritten += bytesWritten;
	}
	_totalCgiOutputSize += totalBytesWritten;

	if (static_cast<size_t>(bytesRead) < sizeof(buffer))
	{
		close(_cgiOutputCompleteFd);
		_cgiReadFromOutputComplete = true;
		return (CGI_READING_FROM_OUTPUT_DONE);
	}

	return (CGI_RUNNING);
}

CgiHandler::CgiState	CgiHandler::handleCgiOutputHeadersIncomplete(char readBuffer[], size_t bytesRead, size_t readBufferSize)
{
	_cgiOutputBuffer.append(readBuffer, bytesRead);
	
	_cgiParsingState = findHeadersEnd(readBuffer, bytesRead, readBufferSize);
	if (_cgiParsingState == PARSING_HEADERS_INVALID)
		return (errorRunning(502, "Headers end not found", CGI_RUNNING_ERROR));
	else if (_cgiParsingState == PARSING_HEADERS_NEED_DATA)
		return (CGI_RUNNING);
	
	_cgiParsingState = parseHeaders();
	if (_cgiParsingState == PARSING_HEADERS_INVALID)
		return (errorRunning(502, "Parsing CGI headers invalid", CGI_RUNNING_ERROR));
	
	_cgiOutputHeadersParsingComplete = true;
	
	ssize_t	remainingBytesToWrite = static_cast<ssize_t>(_cgiOutputBuffer.size() - (_cgiHeadersEndIndex + 4));
	if (remainingBytesToWrite == 0 && bytesRead < readBufferSize)
		return (CGI_READING_FROM_OUTPUT_DONE);

	const char*	remainingBytes = _cgiOutputBuffer.c_str() + _cgiHeadersEndIndex + 4;
	ssize_t	totalBytesWritten = 0;
	while (totalBytesWritten < remainingBytesToWrite)
	{
		ssize_t bytesWritten = write(_cgiOutputCompleteFd, remainingBytes + totalBytesWritten, remainingBytesToWrite - totalBytesWritten);
		if (bytesWritten < 0)
			return (errorRunning(502, "Failed to write from buffer into cgi complete output tmp file", CGI_RUNNING_ERROR));

		totalBytesWritten += bytesWritten;
	}
	_totalCgiOutputSize += totalBytesWritten;
	if (bytesRead < readBufferSize)
	{
		close(_cgiOutputCompleteFd);
		_cgiReadFromOutputComplete = true;
		return (CGI_READING_FROM_OUTPUT_DONE);
	}
	return (CGI_RUNNING);
}

CgiHandler::CgiParsingState	CgiHandler::findHeadersEnd(char readBuffer[], size_t bytesRead, size_t readBufferSize)
{
	_cgiHeadersEndIndex = _cgiOutputBuffer.find("\r\n\r\n");		
	
	if (_cgiHeadersEndIndex == TStr::npos)		// CRLF CRLF marking headers end not found
	{	
		if (_cgiOutputBuffer.size() > HttpRequestData::_maxSizeRequestAndHeaders)
			return (errorParsing("Headers too long", PARSING_HEADERS_INVALID)); // \r\n not found but max capacity reached

		if (bytesRead < readBufferSize)
			return (errorParsing("EOF reached before headers end", PARSING_HEADERS_INVALID)); // \r\n not found but max capacity reached

		_cgiHeadersEndIndex = _cgiOutputBuffer.size() < 3 ? 0 : _cgiOutputBuffer.size() ;
		return (PARSING_HEADERS_NEED_DATA);
	}
	return (PARSING_HEADERS_END_FOUND);
}

CgiHandler::CgiParsingState	CgiHandler::parseHeaders(void)
{
	while (true)
	{
		size_t	lineEnd = _cgiOutputBuffer.find("\r\n", _index);		
		size_t	lineSize = lineEnd - _index;
		
		if (lineSize == 0)		// CRLF found on an empty line -> end of headers
		{
			_index += 2;
			return (PARSING_HEADERS_DONE);
		}

		if (lineSize > HttpRequestData::_maxSizeHeaderLine)
			return (errorParsing("Parsing headers - Header size too long", PARSING_HEADERS_INVALID));

		_cgiHeadersCount++;
		if (_cgiHeadersCount > HttpRequestData::_maxHeaderCount)
			return (errorParsing("Parsing headers - Too many headers", PARSING_HEADERS_INVALID));

		if (parseHeaderLine(_cgiOutputBuffer.substr(_index, lineSize)) == PARSING_HEADERS_INVALID)
			return (PARSING_HEADERS_INVALID);
		
		_cgiHeadersSize = 0;
		_index = lineEnd + 2;
	}
}

CgiHandler::CgiParsingState		CgiHandler::parseHeaderLine(const TStr& headerLine)
{
	size_t	colon = headerLine.find(':');

	if (colon == 0 || colon == std::string::npos)
		return (errorParsing("Parsing header line", PARSING_HEADERS_INVALID));
	if (colon >= HttpRequestData::_maxSizeHeaderName)
		return (errorParsing("Parsing header line, header name too long", PARSING_HEADERS_INVALID));

	TStr	headerName;
	headerName.reserve(colon);
	for (size_t i = 0; i < colon; i++)
		headerName += std::tolower(headerLine[i]);
	TStr	headerValue = trimHeadAndTail(headerLine.substr(colon + 1));
	_cgiOutputHeaders[headerName] = headerValue;
	return (PARSING_HEADERS_PROCESSING);
}



CgiHandler::CgiState	CgiHandler::getCgiState(void) const { return _cgiState; }
int						CgiHandler::getCgiStatusCode(void) const { return _cgiStatusCode; }
bool					CgiHandler::isOutOnly(void) const { return _outOnly; }
pid_t					CgiHandler::getCgiPid(void) const { return _pid; }
int						CgiHandler::getInputPipeWrite(void) const { return _inputPipeFd[1]; }
int						CgiHandler::getOutputPipeRead(void) const { return _outputPipeFd[0]; }
int						CgiHandler::getRequestBodyInputFd(void) const { return _requestBodyInputFd; }
int						CgiHandler::getCgiCompleteOutputFd(void) const { return _cgiOutputCompleteFd; }
const TStr&				CgiHandler::getCgiCompleteOutputFilename(void) const { return _cgiOutputCompleteFilepath; }
int						CgiHandler::getTotalCgiOutputSize(void) const
{ 
	if (_totalCgiOutputSize > INT_MAX)
		return (INT_MAX);
	return static_cast<int>(_totalCgiOutputSize); 

}
const std::map<TStr, TStr>&	CgiHandler::getCgiOutputHeaders(void) const { return _cgiOutputHeaders; }
bool					CgiHandler::isCgiReadFromOutputComplete(void) const { return _cgiReadFromOutputComplete; }

void					CgiHandler::setOutOnly(bool val) { _outOnly = val; }

size_t		CgiHandler::getActualBytesWrittenToCgiInput(void) const { return _actualBytesWrittenToCgiInput; }
size_t		CgiHandler::getActualBytesReadFromCgiOutput(void) const { return _actualBytesReadFromCgiOutput; }