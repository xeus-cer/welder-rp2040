#ifndef __LOG_HPP
#define __LOG_HPP

#include <string>
#include <iostream>
#include <sstream>
#include "pico/time.h"
#include "stdio.h"
#include <cstdlib>
#include <cstring>

// log level is set in CMakeLists.txt
// #define _LOG_LEVEL 1 = ERROR
// #define _LOG_LEVEL 2 = WARNING
// #define _LOG_LEVEL 3 = INFO
// #define _LOG_LEVEL 4 = DEBUG

// macro to strip file path from __FILE__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)


#ifdef NDEBUG
// do not log in release mode
#define xlog(level, msg) do { } while (0)
#else
// define xerxes_log(level, msg) to printf as a macro in format
// [Time][Log level] — [File]:[Line] [Function] — [Text]
// e.g. [0.000000][INFO] — [src/main.cpp:123 main] — Hello World!
// sleep_ms(1) is used to prevent the log messages from being printed in the
// wrong order, enough for 200 bytes with baudrate 1500000
#define xlog(level, msg) do { \
    float timestamp = time_us_64() / 1000000.0f; \
    std::cout << "[" << timestamp << "][" << level << "] — [" << __FILENAME__ << ":" << __LINE__ << " " << __func__ << "()] — " << msg << std::endl; \
    sleep_ms(1); \
} while (0)
#endif // NDEBUG


// always log errors
#define xlog_error(msg) do { \
    xlog("ERROR", msg); \
} while (0)

// convenience macros
#define xlog_err(msg) xlog_error(msg)
#define xloge(msg) xlog_error(msg)

// log warnings if log level is warning or higher
#if (_LOG_LEVEL >= 2)
#define xlog_warning(msg) do { \
    xlog("WARNING", msg); \
} while (0)
#else
#define xlog_warning(msg) do { } while (0)
#endif // _LOG_LEVEL

// convenience macros
#define xlog_warn(msg) xlog_warning(msg)
#define xlogw(msg) xlog_warning(msg)


#if (_LOG_LEVEL >= 3)
#define xlog_info(msg) do { \
    xlog("INFO", msg); \
} while (0)
#else
#define xlog_info(msg) do { } while (0)
#endif // _LOG_LEVEL

// convenience macros
#define xlogi(msg) xlog_info(msg)

// log debug messages if log level is debug
#if (_LOG_LEVEL >= 4)
#define xlog_debug(msg) do { \
    xlog("DEBUG", msg); \
} while (0)
#else
#define xlog_debug(msg) do { } while (0)
#endif // _LOG_LEVEL

// convenience macros
#define xlogd(msg) xlog_debug(msg)
#define xlog_dbg(msg) xlog_debug(msg)


#endif    /* LOG_HPP */