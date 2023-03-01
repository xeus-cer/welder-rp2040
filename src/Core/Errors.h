#ifndef __ERROR_H
#define __ERROR_H

#include <stdint.h>


#ifdef	__cplusplus
extern "C" {
#endif


enum class ERROR_NUM {
    UART_OVERLOAD           = 0,
    CPU_OVERLOAD            = 1,
    BUS_COLLISION           = 2,
    WATCHDOG_TIMEOUT        = 3,
    SENSOR_OVERLOAD         = 4,
    SENSOR_CONNECTION       = 5,
    SENSOR_CONNECTION_MEM   = 6
};


/**
 * @brief Error codes for the error register
 * 
 */
enum : uint64_t
{
    ERROR_MASK_UART_OVERLOAD            = 1 << static_cast<uint64_t>(ERROR_NUM::UART_OVERLOAD),
    ERROR_MASK_CPU_OVERLOAD             = 1 << static_cast<uint64_t>(ERROR_NUM::CPU_OVERLOAD),
    ERROR_MASK_BUS_COLLISION            = 1 << static_cast<uint64_t>(ERROR_NUM::BUS_COLLISION),
    ERROR_MASK_WATCHDOG_TIMEOUT         = 1 << static_cast<uint64_t>(ERROR_NUM::WATCHDOG_TIMEOUT),
    ERROR_MASK_SENSOR_OVERLOAD          = 1 << static_cast<uint64_t>(ERROR_NUM::SENSOR_OVERLOAD),
    ERROR_MASK_SENSOR_CONNECTION        = 1 << static_cast<uint64_t>(ERROR_NUM::SENSOR_CONNECTION),
    ERROR_MASK_SENSOR_CONNECTION_MEM    = 1 << static_cast<uint64_t>(ERROR_NUM::SENSOR_CONNECTION_MEM)
};

#ifdef	__cplusplus
}
#endif

#endif // __ERROR_H