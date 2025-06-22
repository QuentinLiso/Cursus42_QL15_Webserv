/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7C_CgiHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/19 16:19:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/22 20:33:10 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "7C_CgiHandler.hpp"

CgiHandler::CgiHandler(void) 
		:	_cgiState(CGI_SETUP),
			_cgiStatusCode(200),
			_outOnly(true),
			_pid(-1),

			_requestBodyInputFd(-1),
			_cgiWriteOffset(0),
			_cgiWriteToInputComplete(false),
			_actualBytesWrittenToCgiInput(0),

			_cgiOutputCompleteFd(-1),
			_cgiReadOffset(0),
			_cgiReadFromOutputComplete(false),
			_actualBytesReadFromCgiOutput(0),
			_cgiHeadersEnd(false),
			_index(0)
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
		int devnull = open("/dev/null", O_RDONLY);
		if (devnull >= 0)
		{
			dup2(devnull, STDIN_FILENO);
			close(devnull);
		}
		else
		{
			close(STDIN_FILENO);
		}
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


bool	CgiHandler::findHeadersEnd(void)
{
	size_t	limit = std::min(_cgiReadOutputBuffer.size(), 4096UL);
	size_t	headersEnd = TStr::npos;
	for (size_t i = 0; i + 3 < limit; i++)
	{
		if (_cgiReadOutputBuffer[i] == '\r' && _cgiReadOutputBuffer[i + 1] == '\n' && _cgiReadOutputBuffer[i + 2] == '\r' && _cgiReadOutputBuffer[i + 3] == '\n')
		{
			std::cout << "Headers end found : " << i << std::endl;
			headersEnd = i;
			break ;
		}
	}

	if (headersEnd != TStr::npos)
	{
		// Headers End found within the limit :) Try parsing
		parsingOutputHeaders();
		_cgiHeadersEnd = true;
		_cgiReadOutputBuffer.erase(0, headersEnd + 4);
		return (true);
	}
	else if (headersEnd == TStr::npos && _cgiReadOutputBuffer.size() < 4096UL)
	{
		// Headers end not found but output buffer size is less than max size -> try again next round
		return (false);
	}
	else
	{
		// Headers end not found and buffer bigger than max size -> consider no headers
		_cgiHeadersEnd = true;
		return (true);
	}
}

void	CgiHandler::parsingOutputHeaders(void)
{
	std::cout << "Parsing output headers : " << std::endl;
	std::cout << _cgiReadOutputBuffer.substr(0, _cgiHeadersEnd) << std::endl;
	while (_index < _cgiHeadersEnd)
	{
		size_t	lineEnd = _cgiReadOutputBuffer.find("\n", _index);
		size_t	lineSize = lineEnd - _index;	// Line == "Something\n", _index = 'S' -> 0, lineEnd = '\n' -> 9, lineSize = 9

		TStr	headerLine = _cgiReadOutputBuffer.substr(_index, lineEnd - _index);
		std::cout << "Header line : " << headerLine << std::endl;
		size_t	colon = headerLine.find(':');
		if (colon == 0 || colon == TStr::npos || colon == headerLine.size() - 1)
		{
			_index += lineSize + 1;
			continue ;
		}
		TStr	key = trimHeadAndTail(headerLine.substr(0, colon));
		TStr	value = trimHeadAndTail(headerLine.substr(colon + 1));
		
		if (key == "Status")
		{
			std::istringstream iss(value);
			int status;
			if (!(iss >> status))
				status = 200;
			_cgiStatusCode = status;
		}
		else
			_cgiOutputHeaders[key] = value;
		_index += lineSize + 1;
	}
}



CgiHandler::CgiState	CgiHandler::setupCgi(const HttpRequest& httpRequest, const HttpRequestResolution& httpResolution)
{
	return (_outOnly ? setupCgiOutput(httpRequest, httpResolution) : setupCgiInputOutput(httpRequest, httpResolution));
}

bool	CgiHandler::writeToCgiInputPipe(void)
{
	char buffer[1024 * 24];
	
	while (!_cgiWriteInputBuffer.empty())
	{
		ssize_t	bytesWritten = write(_inputPipeFd[1], _cgiWriteInputBuffer.c_str(), _cgiWriteInputBuffer.size());
		if (bytesWritten <= 0)
		{
			return (false);
		}
		_cgiWriteInputBuffer.erase(0, bytesWritten);
		_actualBytesWrittenToCgiInput += bytesWritten;
	}

	ssize_t	bytesRead = read(_requestBodyInputFd, buffer, sizeof(buffer));
	if (bytesRead < 0)
	{
		return (false);
	}
	else if (bytesRead == 0)
	{
		_cgiWriteToInputComplete = true;
		return (true);
	}
	_cgiWriteInputBuffer.append(buffer, bytesRead);
	return (false);
}

bool	CgiHandler::readFromCgiOutputPipe(void)
{
	char buffer[1024 * 24];

	ssize_t	bytesRead = read(_outputPipeFd[0], buffer, sizeof(buffer));

	if (bytesRead < 0)
		return (false);
	if (bytesRead > 0)
		_cgiReadOutputBuffer.append(buffer, bytesRead);
	
	if (!_cgiHeadersEnd)
		findHeadersEnd();
	
	if (_cgiHeadersEnd)
		flushBuffer();

	if (bytesRead == 0)
	{
		_cgiReadFromOutputComplete = true;
		return (true);
	}
	return (false);
}

bool	CgiHandler::flushBuffer(void)
{
	while (!_cgiReadOutputBuffer.empty())
	{
		ssize_t	bytesWritten = write(_cgiOutputCompleteFd, _cgiReadOutputBuffer.c_str(), _cgiReadOutputBuffer.size());
		if (bytesWritten <= 0)
		{
			return (false);
		}
		_cgiReadOutputBuffer.erase(0, bytesWritten);
		_actualBytesReadFromCgiOutput += bytesWritten;
	}
	return (true);
}

CgiHandler::CgiState	CgiHandler::getCgiState(void) const { return _cgiState; }
int						CgiHandler::getCgiStatusCode(void) const { return _cgiStatusCode; }
bool					CgiHandler::isOutOnly(void) const { return _outOnly; }
void					CgiHandler::setOutOnly(bool val) { _outOnly = val; }

pid_t					CgiHandler::getCgiPid(void) const { return _pid; }
int						CgiHandler::getInputPipeWrite(void) const { return _inputPipeFd[1]; }
int						CgiHandler::getOutputPipeRead(void) const { return _outputPipeFd[0]; }

int						CgiHandler::getRequestBodyInputFd(void) const { return _requestBodyInputFd; }
bool					CgiHandler::isCgiWriteFromInputComplete(void) const { return _cgiWriteToInputComplete; }
size_t					CgiHandler::getActualBytesWrittenToCgiInput(void) const { return _actualBytesWrittenToCgiInput; }

const TStr&				CgiHandler::getCgiCompleteOutputFilename(void) const { return _cgiOutputCompleteFilepath; }
int						CgiHandler::getCgiCompleteOutputFd(void) const { return _cgiOutputCompleteFd; }
const std::map<TStr, TStr>&	CgiHandler::getCgiOutputHeaders(void) const { return _cgiOutputHeaders; }
bool					CgiHandler::isCgiReadFromOutputComplete(void) const { return _cgiReadFromOutputComplete; }
size_t					CgiHandler::getActualBytesReadFromCgiOutput(void) const { return _actualBytesReadFromCgiOutput; }