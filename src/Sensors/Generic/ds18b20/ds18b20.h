/* 
 * File:   ds18b20.h
 * Author: themladypan
 *
 * Created on March 27, 2023, 12:44 PM
 */

#ifndef DS18B20_H
#define	DS18B20_H


#include "onewire.h"
#include "Utils/Log.h"
#include <sstream>

#ifdef	__cplusplus
extern "C" {
#endif


/// @brief Write a byte to the 1-Wire bus, takes approx 2.5ms
/// @param pin 
/// @param write_data 
/// @return 
bool _writeAll(uint pin, char write_data)
{
    // reset bus takes approx 1ms
    bool present = OWReset(pin);
    // skip ROM

    // write 0xCC command to skip ROM - takes approx 70*8us = 560us
    OWWriteByte(pin, 0xCC);
    // read Temp, takes approx 750us
    OWWriteByte(pin, write_data);
    return present;
}


/**
 * @brief Start measurement of all sensors on the line, takes approx 2.5ms
 * 
 * @param pin 
 * @return true 
 * @return false 
 */
bool startAll(uint pin)
{
    // write 0x44 command to start measurement of all sensors on the line
    return _writeAll(pin, 0x44);
}


/**
 * @brief Read temperature from all sensors on the line, takes approx 5ms
 * 
 * @param pin 
 * @return double 
 */
double readJustOneTemp(uint pin)
{
    // write to all sensors 0xBE, must be just one connected.
    _writeAll(pin, 0xBE);
    std::stringstream ss;

    char tempL = OWReadByte(pin);
    char tempH = OWReadByte(pin);
    // log values as decimal
    ss << "tempL: " << (int)tempL << " tempH: " << (int)tempH;
    xlog_debug(ss.str());
    // convert low and high bytes to int16
    int16_t raw_temp = (tempH << 8) | tempL;
    
    return 0.0625*raw_temp;
}

#ifdef	__cplusplus
}
#endif

#endif	/* DS18B20_H */
