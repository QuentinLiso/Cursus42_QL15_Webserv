/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Console.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 09:52:48 by qliso             #+#    #+#             */
/*   Updated: 2025/05/19 00:57:56 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Console.hpp"

// STATIC VARIABLES
bool                                            Console::_consoleState = DEFAULT_CONSOLE_STATE;
bool                                            Console::_logFileState = DEFAULT_LOG_FILE_STATE;
bool                                            Console::_logDebugState = DEFAULT_LOG_DEBUG_STATE;
std::string										Console::_configFileName = "";
std::string                              Console::_logFileName = Console::_generateLogFileName();
std::map<Console::LogLevel, std::string> Console::_logLevelStr = Console::_generateLogLevelStr();
std::map<Console::LogLevel, std::string> Console::_logLevelColor = Console::_generateLogLevelColor();


// PRIVATE FUNCTIONS
std::string Console::_generateLogFileName(void)
{
    std::time_t time = std::time(NULL);
    char        buffer[80];
    std::strftime(buffer, sizeof(buffer), "webserv_%Y-%m-%d_%H-%M-%S.log", std::localtime(&time));
    return (std::string(buffer));
}

std::map<Console::LogLevel, std::string>    Console::_generateLogLevelStr(void)
{
    std::map<Console::LogLevel, std::string>    logLevelStr;

    logLevelStr[Console::FATAL] = "FATAL";
    logLevelStr[Console::ERROR] = "ERROR";
    logLevelStr[Console::WARNING] = "WARNING";
    logLevelStr[Console::INFO] = "INFO";
    logLevelStr[Console::TRACE] = "TRACE";
    logLevelStr[Console::DEBUG] = "DEBUG";

    return (logLevelStr);
}

std::map<Console::LogLevel, std::string>    Console::_generateLogLevelColor(void)
{
    std::map<Console::LogLevel, std::string>    logLevelColor;

    logLevelColor[Console::FATAL] = C_RED;
    logLevelColor[Console::ERROR] = C_RED;
    logLevelColor[Console::WARNING] = C_YELLOW;
    logLevelColor[Console::INFO] = C_GREEN;
    logLevelColor[Console::TRACE] = C_MAGENTA;
    logLevelColor[Console::DEBUG] = C_CYAN;

    return (logLevelColor);
}

std::string    Console::_formatter(Console::LogLevel level, const std::string& msg, std::string time, bool colored)
{
    std::string formattedMsg;

    if (colored)
        formattedMsg += Console::getLogLevelColor(level);
    formattedMsg += "[" + Console::getLogLevelStr(level) + "]\t" + time + " : " + msg;
    if ((level == Console::FATAL) && errno != 0)
        formattedMsg += " : " + static_cast<std::string>(std::strerror(errno));
    if (colored)
        formattedMsg += C_RESET;

    return (formattedMsg);
}

void    Console::_printLog(Console::LogLevel level, const std::string& msg, std::string time)
{
    std::cout << Console::_formatter(level, msg, time) << std::endl;
}

void    Console::_writeLogInFile(Console::LogLevel level, const std::string& msg, std::string time)
{
    if (mkdir("logs", 0777) == -1 && errno != EEXIST)
    {
        std::cerr << "Error: " << std::strerror(errno) << std::endl;
        return ;
    }
    std::string         filename = "logs/" + Console::getLogFileName();
    std::ofstream    file(filename.c_str(), std::ofstream::out | std::ofstream::app);
    if (!file.is_open())
    {
        std::cerr << "Error: " << std::strerror(errno) << std::endl;
        return ;
    }
    file << Console::_formatter(level, msg, time, false) << std::endl;
    file.close();
}


// GETTERS & SETTERS
bool    Console::getConsoleState(void) { return Console::_consoleState; }
void    Console::setConsoleState(bool state) { Console::_consoleState = state; }

bool    Console::getLogFileState(void) { return Console::_logFileState; }
void    Console::setLogFileState(bool state) { Console::_logFileState = state; }

bool    Console::getLogDebugState(void) { return Console::_logDebugState; }
void    Console::setLogDebugState(bool state) { Console::_logDebugState = state; }

const std::string&	Console::getConfigFileName(void) { return Console::_configFileName; };
void				Console::setConfigFileName(const std::string& fileName) { Console::_configFileName = fileName; };

std::string Console::getLogFileName(void) { return Console::_logFileName; }
std::string Console::getLogLevelStr(Console::LogLevel level) { return Console::_logLevelStr.at(level); }
std::string Console::getLogLevelColor(Console::LogLevel level) { return Console::_logLevelColor.at(level); }


// PUBLIC FUNCTIONS
void    Console::log(Console::LogLevel level, const std::string& msg)
{
    if (!Console::_consoleState || (level == Console::DEBUG && !Console::_logDebugState))
        return ;
    
    std::time_t time = std::time(NULL);
    char    buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));

    Console::_printLog(level, msg, buffer);

    if (Console::_logFileState)
        Console::_writeLogInFile(level, msg, buffer);
    
    if (level == Console::FATAL)
        throw std::runtime_error(msg);
}

bool    Console::configLog(	Console::LogLevel level, 
							int line,
							int column,
							const std::string& configStep,
							const std::string& msg,
							const std::string& explicitLine,
							bool valid)
{

	std::ostringstream	oss;

	oss << getLogLevelColor(level) << C_BOLD
		<< getLogLevelStr(level) << ": "
		<< _configFileName << " "
		<< "(l" << line << ":c" << column << "): "
		<< C_RESET
		<< configStep << ": "
		<< msg;

	if (!explicitLine.empty())
		oss << " '" << explicitLine << "'";

	std::cout << oss.str() << std::endl;
	return (valid);
}

bool    Console::configLog(	Console::LogLevel level, 
							int line,
							int column,
							const std::string& configStep,
							const std::string& msg,
							const std::vector<std::string>& explicitLine,
							bool valid)
{
	std::ostringstream	oss;

	oss << getLogLevelColor(level) << C_BOLD
		<< getLogLevelStr(level) << ": "
		<< _configFileName << " "
		<< "(l" << line << ":c" << column << "): "
		<< C_RESET
		<< configStep << ": "
		<< msg
		<< " '";
	for (size_t i = 0; i < explicitLine.size(); i++)	
		oss << explicitLine[i];
	oss <<  "'";

	std::cout << oss.str() << std::endl;
    if (level == Console::FATAL)
	{
		throw std::runtime_error(oss.str());
	}
	return (valid);
}

bool	Console::error(const Directive* directive, const std::string& configStep, const std::string& msg)
{
	int line;
	int column;
	std::ostringstream oss;
	
	if (directive)
	{
		line = directive->line;
		column = directive->column;
		
		std::vector<std::string>	args = directive->arguments;
		
		oss << directive->name;
		for (size_t i = 0; i < args.size(); i++)	
			oss << " " << args[i];
	}
	else
	{
		line = 0;
		column = 0;
		oss << "";
	}
	configLog(ERROR, line, column, configStep, msg, oss.str());
	return (false);
}

bool	Console::error(const Block* block, const std::string& configStep, const std::string& msg)
{
	int line;
	int column;
	std::ostringstream oss;
	
	if (block)
	{
		line = block->line;
		column = block->column;
		
		std::vector<std::string>	args = block->arguments;
		
		oss << block->name;
		for (size_t i = 0; i < args.size(); i++)	
			oss << " " << args[i];
	}
	else
	{
		line = 0;
		column = 0;
		oss << "";
	}
	configLog(ERROR, line, column, configStep, msg, oss.str());
	return (false);
}

void	Console::warning(const Directive* directive, const std::string& configStep, const std::string& msg)
{
	if (!directive)
		return ;

	std::ostringstream oss;
	std::vector<std::string>	args = directive->arguments;
	
	oss << directive->name;
	for (size_t i = 0; i < args.size(); i++)	
		oss << " " << args[i];
	configLog(WARNING, directive->line, directive->column, configStep, msg, oss.str());
}

void	Console::warning(const Block* block, const std::string& configStep, const std::string& msg)
{
	if (!block)
		return ;

	std::ostringstream oss;
	std::vector<std::string>	args = block->arguments;
	
	oss << block->name;
	for (size_t i = 0; i < args.size(); i++)	
		oss << " " << args[i];
	configLog(WARNING, block->line, block->column, configStep, msg, oss.str());
}