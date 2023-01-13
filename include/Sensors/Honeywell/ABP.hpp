#ifndef __ABP_HPP
#define __ABP_HPP


#include "hardware/spi.h"
#include "Sensors/Sensor.hpp"
#include <array>
#include "xerxes_rp2040.h"


namespace Xerxes
{

// sensor specific variables
const float VALmin  = 1638.0;   // counts = 10% 2^14
const float VALmax  = 14745.0;  // counts = 90% 2^14
const float Pmin    = 0.0;      // mbar    
const float Pmax    = 60.0;    // mbar, or: 611.8298 mm

class ABP : public Sensor
{
private:
    double pv0;    
    double pv1;
    double pv2;
    double pv3;

public:
    using Sensor::Sensor;
    void init();
    void update();
    void read(std::array<float*, 4> pvs);
    void stop();
};


void ABP::init()
{    
    // init spi with freq 800kHz, return actual frequency
    uint baudrate = spi_init(spi0, 800'000);

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
    this->update();
}


void ABP::update()
{
    uint8_t data[4];
    // uint8_t b0, b1, t0, t1;
    uint16_t p_val;
    uint16_t t_val;
    uint8_t status;

    // SPI_CS active low, read data
    gpio_put(SPI0_CSN_PIN, 0);
    sleep_us(3);
    spi_read_blocking(spi0, 0, data, 4);
    sleep_us(3);
    gpio_put(SPI0_CSN_PIN, 1);

    // convert data, see datasheet
    status = data[0] >> 6;
    p_val = (uint16_t)(((data[0] & 0b00111111)<<8) + data[1]);
    t_val = (uint16_t)(((data[2]<<8) + (data[3] & 0b11100000))>>5);
    
    pv0 = (float)((((p_val-VALmin)*(Pmax-Pmin))/(VALmax-VALmin)) + Pmin); 
    pv3 = (float)((t_val*200.0/2047.0)-50.0);     // calculate internal temperature
}


void ABP::read(std::array<float*, 4> pvs)
{
    *pvs[0] = pv0;
    *pvs[1] = pv1;
    *pvs[2] = pv2;
    *pvs[3] = pv3;
}


void ABP::stop()
{
    gpio_put(EXT_3V3_EN_PIN, false);
    spi_deinit(spi0);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_IN);
    gpio_set_dir(SPI0_CSN_PIN, GPIO_IN);

}

} //namespace Xerxes

#endif // !__ABP_HPP