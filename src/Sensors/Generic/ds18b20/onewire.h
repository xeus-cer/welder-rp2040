/* 
 * File:   onewire.h
 * Author: themladypan
 *
 * Created on March 27, 2023, 12:44 PM
 */

#ifndef ONEWIRE_H
#define	ONEWIRE_H

#include "hardware/gpio.h"
#include "pico/time.h"

#ifdef	__cplusplus
extern "C" {
#endif


void _setLow(uint pin)
{    
    // set pin as output = clear the tris bit
    gpio_set_dir(pin, GPIO_OUT);
    // Set pin Low
    gpio_put(pin, 0);
}


void _setHigh(uint pin)
{
    // set pin as output = clear the tris bit
    gpio_set_dir(pin, GPIO_OUT);
    // set pin high
    gpio_put(pin, 1);
}


void _release(uint pin)
{
    // set pin as input
    gpio_set_dir(pin, GPIO_IN);
} 


/**
 * Sample value from pin
 * @param pin
 * @return 
 */
unsigned _getVal(uint pin)
{
    //read pin 
    _release(pin);
    return gpio_get(pin);
}


/**
 * Read a bit from the 1-Wire slaves (Read time slot). Approx. 70us
 * Drive bus low, delay 6 μs.
 * Release bus, delay 9 μs.
 * Sample bus to read bit from slave.
 * Delay 55 μs.
 * @param pin
 * @return 
 */
unsigned _readBit(uint pin)
{
    _setLow(pin);
    sleep_us(6);
    _release(pin);
    sleep_us(9);
    unsigned sample = _getVal(pin);
    sleep_us(55);
    return sample;
}


/**
 * Send '0' bit to the 1-Wire slaves (Write 0 slot time) Approx. 70us
 * Drive bus low, delay 60 μs.
 * Release bus, delay 10 μs.
 * @param pin
 */
void _write0(uint pin)
{
    _setLow(pin);
    sleep_us(60);
    _release(pin);
    sleep_us(10);            
}

/**
 * Send '1' bit to the 1-Wire slaves (Write 1 slot time) Approx. 70us
 * Drive bus low, delay 6 μs.
 * Release bus, delay 64 μs.
 * @param pin
 */
void _write1(uint pin)
{
    _setLow(pin);
    sleep_us(6);
    _release(pin);
    sleep_us(64);          
}


/**
 * Write bit to bus
 * @param pin
 * @param data - either 0/1
 */
void _writeBit(uint pin, unsigned data)
{
    if(data)
    {
        _write1(pin);
    }
    else
    {
        _write0(pin);
    }
}


/**
 * Read up to a byte of data
 * @param pin
 * @return 
 */
char OWReadByte(uint pin)
{
    unsigned char loop;
    unsigned char result = 0;
    for (loop = 0; loop < 8; loop++)
    {
        result >>= 1; // shift the result to get it ready for the next bit to receive
        if (_readBit(pin))
        {
            result |= 0x80; // if result is one, then set MS-bit
        }
    }
    return (result);
}


/**
 * Write a chunk of bits (byte) to the bus
 * @param pin
 * @return 
 */
void OWWriteByte(uint pin, char write_data)
{
    unsigned char loop;
    for (loop = 0; loop < 8; loop++)
    {
        _writeBit(pin, (write_data & 0x01)); //Sending LS-bit first
        write_data >>= 1; // shift the data byte for the next bit to send
    }
}


/**
 *  Reset the 1-Wire bus slave devices and get them ready for a command
 * Drive bus low, delay 480 μs.
 * Release bus, delay 70 μs.
 * Sample bus: 
 *      0 = device(s) present,
 *      1 = no device present
 * Delay 410 μs.
 * @param pin
 * @return 
 */
bool OWReset(uint pin)
{
    unsigned present = 0;
    
    _setLow(pin);    
    sleep_us(480);
    _release(pin);
    sleep_us(70);
    
    // after reset, read bus
    present = _getVal(pin);
    
    sleep_us(410);
    
    //if device is present, return 1
    return !present;    
}


#ifdef	__cplusplus
}
#endif

#endif	/* ONEWIRE_H */
