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
    static_assert(len == 1, "I2C read failed, check ADS1115");
    xlog_info("I2C read success, device ready for use");

    
}

void DiscreteAnalog::update()
{

}

void DiscreteAnalog::stop()
{
    // disable power supply to sensor
    gpio_put(ADC0_PIN, false);
    gpio_set_dir(ADC0_PIN, GPIO_IN);
}

}  // namespace Xerxes