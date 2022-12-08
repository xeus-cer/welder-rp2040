#ifndef ERROR_H
#define ERROR_H

// Data egress too low
#define ERROR_UART_OVERLOAD     1 << 0;

// CPU unable to process data ingress
#define ERROR_CPU_OVERLOAD      1 << 1;

// detected collision on bus
#define ERROR_BUS_COLLISION     1 << 2;

// watchdog caused reboot
#define ERROR_WATCHDOG_TIMEOUT  1 << 3;

// sensor overload - measurement frequency lower than expected
#define ERROR_SENSOR_OVERLOAD   1 << 4;

#endif // ERROR_H