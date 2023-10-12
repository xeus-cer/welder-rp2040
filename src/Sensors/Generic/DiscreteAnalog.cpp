#include "DiscreteAnalog.hpp"
#include "Hardware/Board/xerxes_rp2040.h"
#include "hardware/adc.h"
#include <string>
#include <sstream>
#include "pico/time.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

namespace Xerxes
{

DiscreteAnalog::~DiscreteAnalog()
{
    stop();
}

void DiscreteAnalog::_WriteConfig(const uint16_t config) {
    uint8_t data[3];
    data[0] = 0x01;  // points to config register
    // MSB first
    data[1] = (config >> 8) & 0xFF;
    data[2] = config & 0xFF;
    int len = i2c_write_blocking(PICO_DEFAULT_I2C, 0x48, data, 3, false);
    assert(len == 3);  // check if write was successful
}

void DiscreteAnalog::_SetContinuous() {
    uint16_t config = 0x0000;
    config |= 0b10000100 << 8;
    config |= 0b10000011;
    _WriteConfig(config);
}

uint16_t DiscreteAnalog::_ReadConversionResult() {
    uint8_t data[1] = {0x00};  // points to conversion register
    int len = i2c_write_blocking(PICO_DEFAULT_I2C, 0x48, data, 1, false);
    assert(len == 1);  // check if write was successful

    uint8_t result[2];
    // read result, MSB first
    len = i2c_read_blocking(PICO_DEFAULT_I2C, 0x48, result, 2, false);
    assert(len == 2);  // check if read was successful

    // convert to int16_t
    int16_t resultInt = (result[0] << 8) | result[1];
    xlog_debug("ADS1115 read: " << resultInt);
    return resultInt;
}

void DiscreteAnalog::init()
{
    // enable power supply to sensor
    gpio_init(ADC0_PIN);
    gpio_set_dir(ADC0_PIN, GPIO_OUT);

    gpio_put(ADC0_PIN, true);

    // initialize I2C
    i2c_init(PICO_DEFAULT_I2C, 400000);

    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(I2C0_SDA_PIN, I2C0_SCL_PIN, GPIO_FUNC_I2C));

    xlog_info("I2C initialized");

    // read 1 bytes from I2C to check if it works
    uint8_t data[1];
    int len = i2c_read_blocking(PICO_DEFAULT_I2C, 0x48, data, 1, false);
    assert(len == 1);  // check if read was successful
    xlog_info("I2C read success, device ready for use");

    _SetContinuous();
}

void DiscreteAnalog::update() {
    // result is 16 bit from 0 to VREF
    *_reg->pv0 = _ReadConversionResult() * VREF / 65536.0;
}

void DiscreteAnalog::stop()
{
    // disable power supply to sensor
    gpio_put(ADC0_PIN, false);
    gpio_set_dir(ADC0_PIN, GPIO_IN);
}

}  // namespace Xerxes