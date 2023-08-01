#ifndef DS18B20_HPP
#define DS18B20_HPP

#include "Sensors/Sensor.hpp"

namespace Xerxes
{

class DS18B20: public Sensor
{
private:
    /// @brief convenience typedef
    typedef Sensor super;

    uint temp_channel_0 = ADC0_PIN; //26
    uint temp_channel_1 = 27;
    uint temp_channel_2 = 28;
    uint temp_channel_3 = 29;

    constexpr static uint32_t _updateRateHz = 1;  // update frequency in Hz
    constexpr static uint32_t _updateRateUs = _usInS / _updateRateHz;  // update rate in microseconds
    int numChannels;

public:
    using Sensor::Sensor;
    
    void init(int num_channels);
    void init();
    void update();
    void stop();
        
    /**
     * @brief Get the Json object representing the sensor values
     * 
     * @return std::string 
     */
    std::string getJson();

    std::string getJson(uint8_t channel);
    std::string getJsonLast();
    std::string getJsonMin();
    std::string getJsonMax();
    std::string getJsonMean();
    std::string getJsonStdDev();


    void _setLow(uint pin);


    void _setHigh(uint pin);


    void _release(uint pin);

    /**
     * Sample value from pin
     * @param pin
     * @return 
     */
    unsigned _getVal(uint pin);


    /**
     * Read a bit from the 1-Wire slaves (Read time slot). Approx. 70us
     * Drive bus low, delay 6 μs.
     * Release bus, delay 9 μs.
     * Sample bus to read bit from slave.
     * Delay 55 μs.
     * @param pin
     * @return 
     */
    unsigned _readBit(uint pin);

    /**
     * Send '0' bit to the 1-Wire slaves (Write 0 slot time) Approx. 70us
     * Drive bus low, delay 60 μs.
     * Release bus, delay 10 μs.
     * @param pin
     */
    void _write0(uint pin);

    /**
     * Send '1' bit to the 1-Wire slaves (Write 1 slot time) Approx. 70us
     * Drive bus low, delay 6 μs.
     * Release bus, delay 64 μs.
     * @param pin
     */
    void _write1(uint pin);


    /**
     * Write bit to bus
     * @param pin
     * @param data - either 0/1
     */
    void _writeBit(uint pin, unsigned data);


    /**
     * Read up to a byte of data
     * @param pin
     * @return 
     */
    char OWReadByte(uint pin);

    /**
     * Write a chunk of bits (byte) to the bus
     * @param pin
     * @return 
     */
    void OWWriteByte(uint pin, char write_data);

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
    bool OWReset(uint pin);


    /// @brief Write a byte to the 1-Wire bus, takes approx 2.5ms
    /// @param pin 
    /// @param write_data 
    /// @return 
    bool _writeAll(uint pin, char write_data);

    /**
     * @brief Start measurement of all sensors on the line, takes approx 2.5ms
     * 
     * @param pin 
     * @return true 
     * @return false 
     */
    bool startAll(uint pin);


    /**
     * @brief Read temperature from all sensors on the line, takes approx 5ms
     * 
     * @param pin 
     * @return double 
     */
    double readJustOneTemp(uint pin);
};



} // namespace Xerxes

#endif // DS18B20_HPP