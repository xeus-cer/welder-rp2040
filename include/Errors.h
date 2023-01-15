#ifndef __ERROR_H
#define __ERROR_H

extern "C"
{

enum : uint64_t
{
    ERROR_MASK_UART_OVERLOAD     = 1 << 0,  // 1
    ERROR_MASK_CPU_OVERLOAD      = 1 << 1,  // 2
    ERROR_MASK_BUS_COLLISION     = 1 << 2,  // 4
    ERROR_MASK_WATCHDOG_TIMEOUT  = 1 << 3,  // 8
    ERROR_MASK_SENSOR_OVERLOAD   = 1 << 4   // 16
};

}  // extern "C"

#endif // __ERROR_H