#ifndef __ERROR_H
#define __ERROR_H

extern "C"
{

enum : uint64_t
{
    ERROR_UART_OVERLOAD     = 1 << 0,
    ERROR_CPU_OVERLOAD      = 1 << 1,
    ERROR_BUS_COLLISION     = 1 << 2,
    ERROR_WATCHDOG_TIMEOUT  = 1 << 3,
    ERROR_SENSOR_OVERLOAD   = 1 << 4
};

}  // extern "C"

#endif // __ERROR_H