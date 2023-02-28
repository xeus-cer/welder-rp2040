#include "SCL3300a.hpp"

#include "Hardware/Board/xerxes_rp2040.h"
#include "pico/time.h"
#include "hardware/spi.h"

namespace Xerxes
{


void SCL3300a::init()
{    
    constexpr uint sensor_freq_hz = 70;  // 70Hz
    _devid = DEVID_ACCEL_XYZ;
    // set cycle frequency to 70Hz
    *_reg->desiredCycleTimeUs = 1000000 / sensor_freq_hz;  // 70Hz

    rbpv0 = StatisticBuffer<float>(sensor_freq_hz);
    rbpv1 = StatisticBuffer<float>(sensor_freq_hz);
    rbpv2 = StatisticBuffer<float>(sensor_freq_hz);
    rbpv3 = StatisticBuffer<float>(sensor_freq_hz);

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
    volatile uint32_t received_packet;
    
    sleep_ms(1);
    ExchangeBlock(CMD::Switch_to_bank_0);
    sleep_ms(3);

    ExchangeBlock(CMD::SW_Reset);
    sleep_ms(3);

    ExchangeBlock(CMD::Change_to_mode_2);
    sleep_ms(1);
    // ExchangeBlock(CMD::Enable_ANGLE);
    sleep_ms(15);

    ExchangeBlock(CMD::Read_Status_Summary);
    ExchangeBlock(CMD::Read_Status_Summary);
    ExchangeBlock(CMD::Read_Status_Summary);
    sleep_ms(1);
    ExchangeBlock(CMD::Read_WHOAMI);
    ExchangeBlock(CMD::Read_WHOAMI);

    this->update();
}

void SCL3300a::update()
{
    
    auto packetX = std::make_unique<SclPacket_t>();
    auto packetY = std::make_unique<SclPacket_t>();
    auto packetZ = std::make_unique<SclPacket_t>();
    auto packetT = std::make_unique<SclPacket_t>();

    ExchangeBlock(CMD::Read_ACC_X);
    longToPacket(ExchangeBlock(CMD::Read_ACC_Y), packetX);
    longToPacket(ExchangeBlock(CMD::Read_ACC_Z), packetY);
    longToPacket(ExchangeBlock(CMD::Read_Temperature), packetZ);
    longToPacket(ExchangeBlock(CMD::Read_Status_Summary), packetT);

    *_reg->pv0 = getAccFromPacket(packetX, CMD::Change_to_mode_2);
    *_reg->pv1 = getAccFromPacket(packetY, CMD::Change_to_mode_2);
    *_reg->pv2 = getAccFromPacket(packetZ, CMD::Change_to_mode_2);

    // extract temperature from packet and convert to degrees
    uint16_t raw_temp = (uint16_t)(packetT->DATA_H << 8) + packetT->DATA_L;
    *_reg->pv3 = -273 + (static_cast<float>(raw_temp) / 18.9);

    // if calcStat is true, update statistics
    if(_reg->config->bits.calcStat)
    {
        // insert new values into ring buffer
        rbpv0.insertOne(*_reg->pv0);
        rbpv1.insertOne(*_reg->pv1);
        rbpv2.insertOne(*_reg->pv2);
        rbpv3.insertOne(*_reg->pv3);

        // update statistics
        rbpv0.updateStatistics();
        rbpv1.updateStatistics();
        rbpv2.updateStatistics();
        rbpv3.updateStatistics();

        // update min, max stddev etc...
        rbpv0.getStatistics(_reg->minPv0, _reg->maxPv0, _reg->meanPv0, _reg->stdDevPv0);
        rbpv1.getStatistics(_reg->minPv1, _reg->maxPv1, _reg->meanPv1, _reg->stdDevPv1);
        rbpv2.getStatistics(_reg->minPv2, _reg->maxPv2, _reg->meanPv2, _reg->stdDevPv2);
        rbpv3.getStatistics(_reg->minPv3, _reg->maxPv3, _reg->meanPv3, _reg->stdDevPv3);
        
        *_reg->av0 = *_reg->stdDevPv0 * SQRT2;
        *_reg->av1 = *_reg->stdDevPv1 * SQRT2;
        *_reg->av2 = *_reg->stdDevPv2 * SQRT2;
        
        // calculate normal vector from 3 axis std dev
        double normal_stdev = sqrt(pow(*_reg->stdDevPv0, 2) + pow(*_reg->stdDevPv1, 2) + pow(*_reg->stdDevPv2, 2));
        *_reg->av3 = normal_stdev * SQRT2;
    }
}


double SCL3300a::getAccFromPacket(const std::unique_ptr<SclPacket_t>& packet, const cmd_t mode)
{
    // convert to signed 16 bit from 2's complement 
    int16_t raw = (int16_t)(packet->DATA_H << 8) + (int16_t)packet->DATA_L;
    
    
    double sensitivity = 0.0;
    switch (mode)
    {
    case CMD::Change_to_mode_1: 
        sensitivity = 6000;  // LSB / g
        break;
    
    case CMD::Change_to_mode_2:
        sensitivity = 3000;  // LSB / g
        break;

    case CMD::Change_to_mode_3:
        sensitivity = 12000;  // LSB / g
        break;
    
    case CMD::Change_to_mode_4:
        sensitivity = 12000;  // LSB / g
        break;
    
    default:
        sensitivity = 6000;  // LSB / g aka mode 1
        break;
    }

    return (double)(raw * G / sensitivity);
}


std::ostream& operator<<(std::ostream& os, const SCL3300a& scl)
{
    // output pv values as json
    os << "{" << std::endl;
    os << "\"X\":" << *scl._reg->meanPv0 << "," << std::endl;
    os << "\"Y\":" << *scl._reg->meanPv1 << "," << std::endl;
    os << "\"Z\":" << *scl._reg->meanPv2 << "," << std::endl;
    os << "\"Temp\":" << *scl._reg->meanPv3 << "," << std::endl;

    os << "\"MinX\":" << *scl._reg->minPv0 << "," << std::endl;
    os << "\"MinY\":" << *scl._reg->minPv1 << "," << std::endl;
    os << "\"MinZ\":" << *scl._reg->minPv2 << "," << std::endl;

    os << "\"MaxX\":" << *scl._reg->maxPv0 << "," << std::endl;
    os << "\"MaxY\":" << *scl._reg->maxPv1 << "," << std::endl;
    os << "\"MaxZ\":" << *scl._reg->maxPv2 << "," << std::endl;

    os << "\"StdDevX\":" << *scl._reg->stdDevPv0 << "," << std::endl;
    os << "\"StdDevY\":" << *scl._reg->stdDevPv1 << "," << std::endl;
    os << "\"StdDevZ\":" << *scl._reg->stdDevPv2 << "," << std::endl;

    os << "\"AmplitudeX\":" << *scl._reg->av0 << "," << std::endl;
    os << "\"AmplitudeY\":" << *scl._reg->av1 << "," << std::endl;
    os << "\"AmplitudeZ\":" << *scl._reg->av2 << "," << std::endl;
    os << "\"Amplitude\":" << *scl._reg->av3 << "," << std::endl;

    os << "\"Units\":\"[m.s^-2], [°C]\"" << std::endl;

    os << "}";    

    return os;
}

} // namespace Xerxes