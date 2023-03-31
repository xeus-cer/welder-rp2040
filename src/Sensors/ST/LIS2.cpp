#include "LIS2.hpp"

#include "Hardware/Board/xerxes_rp2040.h"
#include "Core/Errors.h"
#include "hardware/spi.h"
#include "pico/time.h"
#include "Utils/Log.h"

namespace Xerxes
{

void LIS2::init()
{    
    _devid = DEVID_ACCEL_XYZ;

    // init spi with freq 10MHz, return actual frequency
    uint baudrate = spi_init(spi0, 10'000'000);
    xlog_debug("SPI0 baudrate: " << baudrate)

    // Set the GPIO pin mux to the SPI
    gpio_set_function(SPI0_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI0_CLK_PIN, GPIO_FUNC_SPI);

    gpio_init(EXT_3V3_EN_PIN);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_OUT);

    // enable sensor 3V3
    gpio_put(EXT_3V3_EN_PIN, true);

    // CS pin as GPIO
    gpio_init(SPI0_CSN_PIN);
    gpio_set_dir(SPI0_CSN_PIN, GPIO_OUT);

    // read out first sequence - usually rubbish anyway
    sleep_ms(10);
    this->update();
}


void LIS2::update()
{
    // CS pin low
    gpio_put(SPI0_CSN_PIN, false);

    // send read command
    spi_write_blocking(spi0, (uint8_t[]){0x80 | 0x28}, 1);

    // read 6 bytes
    uint8_t data[6];
    spi_read_blocking(spi0, 0, data, 6);

    // CS pin high
    gpio_put(SPI0_CSN_PIN, true);

    // convert to int16_t
    int16_t x = (data[1] << 8) | data[0];
    int16_t y = (data[3] << 8) | data[2];
    int16_t z = (data[5] << 8) | data[4];

    // convert to m/s^2
    _x = x * 0.000061f;
    _y = y * 0.000061f;
    _z = z * 0.000061f;
}