#include "Utils/Log.hpp"

#ifndef NDEBUG
void xerxes_log(const std::string& level, const std::string& msg)
{
    float timestamp = time_us_64() / 1000000.0f;
    std::cout << "[" << level << "] (" << timestamp << ") - " << msg << std::endl;
}
#endif // NDEBUG

void xerxes_log_error(const std::string& msg)
{
    xerxes_log("ERROR", msg);
}

// log warnings if log level is warning or higher
#if (_LOG_LEVEL >= 2)
void xerxes_log_warning(const std::string& msg)
{
    xerxes_log("WARNING", msg);
}
#endif // _LOG_LEVEL


#if (_LOG_LEVEL >= 3)
void xerxes_log_info(const std::string& msg)
{
    xerxes_log("INFO", msg);
}
#endif // _LOG_LEVEL


// log debug messages if log level is debug
#if (_LOG_LEVEL >= 4)
void xerxes_log_debug(const std::string& msg)
{
    xerxes_log("DEBUG", msg);
}
#endif // _LOG_LEVEL