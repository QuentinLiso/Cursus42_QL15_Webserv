/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Console.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/08 09:52:46 by qliso             #+#    #+#             */
/*   Updated: 2025/05/09 19:31:35 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONSOLE_HPP
# define CONSOLE_HPP

# define DEFAULT_CONSOLE_STATE true
# define DEFAULT_LOG_FILE_STATE false
# define DEFAULT_LOG_DEBUG_STATE false

# include <iostream>
# include <fstream>
# include <string>
# include <map>
# include <vector>
# include <ctime>
# include <cerrno>
# include <cstring>
# include <sys/stat.h>
# include "Colors.hpp"

class Console
{
public:
    enum    LogLevel
    {
        FATAL = 0,
        ERROR,
        WARNING,
        INFO,
        TRACE,
        DEBUG,
    };

private:
    static bool _consoleState;
    static bool _logFileState;
    static bool _logDebugState;

    static std::string  _logFileName;
    static std::map<Console::LogLevel, std::string> _logLevelStr;
    static std::map<Console::LogLevel, std::string> _logLevelColor;

    static std::string                              _generateLogFileName(void);
    static std::map<Console::LogLevel, std::string> _generateLogLevelStr(void);
    static std::map<Console::LogLevel, std::string> _generateLogLevelColor(void);
    
    static std::string                              _formatter(Console::LogLevel level, const std::string& msg, std::string time, bool colored = true);
    static void                                     _printLog(Console::LogLevel level, const std::string& msg, std::string time);
    static void                                     _writeLogInFile(Console::LogLevel level, const std::string& msg, std::string time);

public:
    static void setConsoleState(bool state);
    static void setLogFileState(bool state);
    static void setLogDebugState(bool state);
    static bool getConsoleState(void);
    static bool getLogFileState(void);
    static bool getLogDebugState(void);

    static std::string  getLogFileName(void);
    static std::string  getLogLevelStr(Console::LogLevel level);
    static std::string  getLogLevelColor(Console::LogLevel level);

    static void log(Console::LogLevel level, const std::string& msg);
};


#endif
