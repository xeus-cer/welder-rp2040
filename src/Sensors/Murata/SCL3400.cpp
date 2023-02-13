#include "SCL3400.hpp"


#include "pico/time.h"

namespace Xerxes
{


void SCL3400::init()
{    
    constexpr uint spi_freq = 2 * MHZ;
    // init spi with freq , return actual frequency
    uint baudrate = spi_init(spi0, spi_freq);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

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


    // chip init sequence        
    sleep_ms(3);

    // Write SW Reset command
    ExchangeBlock(CMD::SW_Reset);
    sleep_ms(3);

    /** 
     * Optional: 
     * 
     * Set Measurement mode Mode A (default)
     * 30° / 0.5g full-scale
     * 10 Hz 1st order low pass filter
    */
    // longToPacket(ExchangeBlock(CMD::Change_to_mode_A), packet);
    // sleep_ms(100);
    
    ExchangeBlock(CMD::Read_Status_Summary);
    ExchangeBlock(CMD::Read_Status_Summary);

    auto status = std::make_unique<SclPacket_t>();
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), status);

    // TODO: find out why this is not working, should return RS = NORMAL, but returns RS = ERROR instead
    // assert(status->RS == NORMAL);

    // read out first sequence - usually rubbish anyway
    sleep_ms(10);
    this->update(false);
}


void SCL3400::update(bool calcStats)
{    
    // pv0 = (float)((((p_val-VALmin)*(Pmax-Pmin))/(VALmax-VALmin)) + Pmin); 
    // pv3 = (float)((t_val*200.0/2047.0)-50.0);     // calculate internal temperature
    // declare packet variables
    auto packetX = std::make_unique<SclPacket_t>();
    auto packetY = std::make_unique<SclPacket_t>();
    auto packetT = std::make_unique<SclPacket_t>();
    
    ExchangeBlock(CMD::Read_ACC_X);
    longToPacket(ExchangeBlock(CMD::Read_ACC_Y), packetX);
    longToPacket(ExchangeBlock(CMD::Read_Temperature), packetY);
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), packetT);

    // convert data to angles
    *_reg->pv0 = static_cast<float>(getDegFromPacket(packetX));
    *_reg->pv1 = static_cast<float>(getDegFromPacket(packetY));

    // extract temperature from packet and convert to degrees
    uint16_t raw_temp = (uint16_t)(packetT->DATA_H << 8) + packetT->DATA_L;
    *_reg->pv3 = -273 + (static_cast<float>(raw_temp) / 18.9);

    // calculate statistics
    super::update();
}


double SCL3400::getDegFromPacket(const std::unique_ptr<SclPacket_t>& packet)
{
    union {
        unsigned char data[2];
        int16_t acc;
    } value;

    value.data[0] = packet->DATA_L;
    value.data[1] = packet->DATA_H;

    // calculate acceleration in g
    double acceleration = static_cast<double>(value.acc) / sensitivityModeA;
    
    // convert to degrees using arcsin
    double degrees = asin(acceleration) * 180.0 / M_PI;
    
    return degrees;
}

} //namespace Xerxes


