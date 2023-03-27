#ifndef __LOG_HPP
#define __LOG_HPP

#include <string>
#include <iostream>
#include <sstream>
#include "pico/time.h"

// log level is set in CMakeLists.txt
// #define _LOG_LEVEL 1 = ERROR
// #define _LOG_LEVEL 2 = WARNING
// #define _LOG_LEVEL 3 = INFO
// #define _LOG_LEVEL 4 = DEBUG

#ifdef NDEBUG
// do not log in release mode
#define xerxes_log(level, msg) do { } while (0)
#else
void xerxes_log(const std::string& level, const std::string& msg);
#endif // NDEBUG


// always log errors
void xerxes_log_error(const std::string& msg);

// log warnings if log level is warning or higher
#if (_LOG_LEVEL >= 2)
void xerxes_log_warning(const std::string& msg);
#else
#define xerxes_log_warning(msg) do { } while (0)
#endif // _LOG_LEVEL


#if (_LOG_LEVEL >= 3)
void xerxes_log_info(const std::string& msg);
#else
#define xerxes_log_info(msg) do { } while (0)
#endif // _LOG_LEVEL


// log debug messages if log level is debug
#if (_LOG_LEVEL >= 4)
void xerxes_log_debug(const std::string& msg);
#else
#define xerxes_log_debug(msg) do { } while (0)
#endif // _LOG_LEVEL


#endif    /* LOG_HPP */