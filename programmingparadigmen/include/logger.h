#ifndef LOGGER_H
#define LOGGER_H

/**
 * @file Logger.h
 * @brief A simple logging class that allows you to log messages to a specified output stream.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <typeinfo>

namespace logging {
	
/**
 * @class Logger
 * @brief A class that provides a simple logging mechanism.
 *
 * This class allows you to log messages to a specified output stream, such as the console or a file.
 * You can specify the log level and the output stream when creating a Logger object.
 */
template<bool Enabled = true>
class Logger {
public:
	/**
     * @enum LogLevel
     * @brief An enumeration of possible log levels.
     */
    enum class LogLevel {
        SOURCE,    /**< source information */
        DEBUGGING, /**< debug messages */
        INFO,      /**< informational messages */
        WARNING,   /**< warning messages */
        SEVERE     /**< severe error messages */
    };
private:
    /**
     * @brief Get the current date and time as a string.
     *
     * @return The current date and time as a string in the format "YYYY-MM-DD HH:MM:SS".
     */
    std::string GetCurrentDateTime();

    /**
     * @brief Get the log level as a string.
     *
     * @return The log level as a string.
     */
    std::string GetLogLevelString();

    LogLevel logLevel; /**< The current log level. */

    LogLevel threshold;  /**< The threshold level for logging */

    std::ostream& outStream; /**< The output stream where log messages will be written. */
	
	template<typename T>
    std::string GetVariableInfo(T var, const char* name, const char* file, int line) {
        std::ostringstream out;
        out << name << " = " << var << "  (" << typeid(T).name() << " " << name << ")  [ line " << line << ", file '" << file << "']";
        return out.str();
    }
	
public:

    void setThreshold(LogLevel level);

    /**
     * @brief Constructor that takes an output stream as a parameter.
     *
     * @param out The output stream where log messages will be written.
     * @param threshold The threshold to log messages.
     * @param logLevel The level to log messages.
     */
    Logger(std::ostream& out, LogLevel threshold = LogLevel::DEBUGGING)
    : outStream(out), threshold(threshold), logLevel(LogLevel::DEBUGGING) {}


    /**
     * @brief Overload of the << operator to set the log level.
     *
     * @param level The new log level.
     * @return A reference to the Logger object.
     */
    Logger& operator<<(const LogLevel& level);

    /**
     * @brief Overload of the << operator to log a message.
     *
     * @param value The message to be logged.
     * @return A reference to the Logger object.
     */
    Logger& operator<<(const std::string& value);

    /**
     * @brief Overload of the << operator to handle manipulators such as std::endl.
     *
     * @param pf The manipulator function.
     * @return A reference to the Logger object.
     */
    Logger& operator<<(std::ostream& (*pf)(std::ostream&));

    template<typename T>
    Logger& source(T var, const char* name, const char* file, int line) {
        if (LogLevel::SOURCE >= threshold) {
            outStream << GetCurrentDateTime() << " " << "SOURCE" << ": " 
                      << GetVariableInfo(var, name, file, line) << std::endl;
        }
        return *this;
    }
};

// Specialization for Logger<false> (disabled logging)
template<>
class Logger<false> {
public:
    enum class LogLevel {
		SOURCE,
        DEBUGGING,
        INFO,
        WARNING,
        SEVERE
    };

    Logger(std::ostream&, LogLevel = LogLevel::DEBUGGING) {}  // Empty constructor

    void setThreshold(LogLevel) {}  // No-op
	

    Logger<false>& operator<<(const LogLevel&) { return *this; }
    Logger<false>& operator<<(const std::string&) { return *this; }
    Logger<false>& operator<<(std::ostream& (*)(std::ostream&)) { return *this; }
};

extern Logger<true> logger;

#define source(logger, var) logger.source(var, #var, __FILE__, __LINE__)
} // end of namespace



#endif  // LOGGER_H
