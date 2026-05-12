/**
 * @file Logger.h
 * @brief A simple logging class that allows you to log messages to a specified output stream.
 */

#include <iostream>
#include <string>
#include <ctime>
#include "logger.h"

namespace logging {
template<bool Enabled>
void Logger<Enabled>::setThreshold(LogLevel level) {
    threshold = level;
}

template<bool Enabled>
Logger<Enabled>& Logger<Enabled>::operator<<(const LogLevel& level) {
    this->logLevel = level;
	outStream << GetCurrentDateTime() << " " << GetLogLevelString() << ": ";
    return *this;
}


template<bool Enabled>
Logger<Enabled>& Logger<Enabled>::operator<<(const std::string& value) {
    if (static_cast<int>(this->logLevel) >= static_cast<int>(threshold)) {
	    outStream << value;
    }
    return *this;
}


template<bool Enabled>
Logger<Enabled>& Logger<Enabled>::operator<<(std::ostream& (*pf)(std::ostream&)) {
    if (static_cast<int>(this->logLevel) >= static_cast<int>(threshold)) {
	    outStream << pf;
	}
    return *this;
}


template<bool Enabled>
std::string  Logger<Enabled>::GetCurrentDateTime() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return std::string(buffer);
}


template<bool Enabled>
std::string  Logger<Enabled>::GetLogLevelString() {
        switch (this->logLevel) {
            case LogLevel::SOURCE:
                return "SOURCE";
            case LogLevel::DEBUGGING:
                return "DEBUGGING";
            case LogLevel::INFO:
                return "INFO";
            case LogLevel::WARNING:
                return "WARNING";
            case LogLevel::SEVERE:
                return "SEVERE";
        }
		return "";
    }


	

// Explicit template instantiation to avoid linker errors (optional, but can be necessary depending on your build system)
template class Logger<true>;
template class Logger<false>;


typedef Logger<true> Log;

Logger<true> logger(std::cout, Logger<true>::LogLevel::INFO);
}

