#ifndef __SEN55__HPP__
#define __SEN55__HPP__


#include "Sensors/Sensor.hpp"
#include <ostream>
#include <string>


namespace Xerxes
{

/**
 * @brief Commands for Sensirion SEN55
 * 
 * @see https://sensirion.com/media/documents/6791EFA0/62A1F68F/Sensirion_Datasheet_Environmental_Node_SEN5x.pdf
 */
enum class SenCmd : uint16_t {
    START_MEASUREMENT = 0x0021, // Write Data < 50 ms, Start Measurement
    START_RHT_GAS_ONLY_MEASUREMENT_MODE = 0x0037, // Write Data < 50 ms, Start Measurement in RHT/Gas-Only Measurement Mode
    STOP_MEASUREMENT = 0x0104, // Write Data < 200 ms, Stop Measurement
    READ_DATA_READY_FLAG = 0x0202, // Read/ Write Data < 20 ms, Read Data-Ready Flag
    READ_MEASURED_VALUES = 0x03C4, // Read/ Write Data < 20 ms, Read Measured Values
    READ_WRITE_TEMPERATURE_COMPENSATION_PARAMETERS = 0x60B2, // Read/ Write Data and Parameters < 20 ms, Read/ Write Temperature Compensation Parameters
    READ_WRITE_WARM_START_PARAMETERS = 0x60C6, // Read/ Write Data and Parameters < 20 ms, Read/ Write Warm Start Parameters
    READ_WRITE_VOC_ALGORITHM_TUNING_PARAMETERS = 0x60D0, // Read/ Write Data and Parameters < 20 ms, Read/Write VOC Algorithm Tuning Parameters
    READ_WRITE_NOX_ALGORITHM_TUNING_PARAMETERS = 0x60E1, // Read/ Write Data and Parameters < 20 ms, Read/Write NOx Algorithm Tuning Parameters
    READ_WRITE_RHT_ACCELERATION_MODE = 0x60F7, // Read/ Write Data and Parameters < 20 ms, Read/Write RH/T Acceleration Mode
    READ_WRITE_VOC_ALGORITHM_STATE = 0x6181, // Read/ Write Data and Parameters < 20 ms, Read/Write VOC Algorithm State
    START_FAN_CLEANING = 0x5607, // Write Data < 20 ms, Start Fan Cleaning
    READ_WRITE_AUTO_CLEANING_INTERVAL = 0x8004, // Read/ Write Data and Parameters < 20 ms, Read/Write Auto Cleaning Interval
    READ_PRODUCT_NAME = 0xD014, // Read/ Write Data < 20 ms, Read Product Name
    READ_SERIAL_NUMBER = 0xD033, // Read/ Write Data < 20 ms, Read Serial Number
    READ_FIRMWARE_VERSION = 0xD100, // Read/ Write Data < 20 ms, Read Firmware Version
    READ_DEVICE_STATUS = 0xD206, // Read/ Write Data < 20 ms, Read Device Status
    CLEAR_DEVICE_STATUS = 0xD210, // Write Data < 20 ms, Clear Device Status
    RESET = 0xD304 // Write Data < 100 ms, Reset
};


/**
 * @brief Sensor class for Sensirion SGP40
 * 
 * Using I2C0 on pins 4 and 5 for communication with the SEN55 sensor
 */
class Sen55 : public Sensor
{
private:
    uint8_t _SEN55_ADDR = 0x69;

    bool _data_ready();

    uint8_t _crc_8(uint8_t* data);
    
    int _send_command(uint16_t command);
    int _send_command(SenCmd command);

    int _read_block(uint8_t *two_bytes);

protected:
    // typedef Sensor as super class for easier access
    typedef Sensor super;

public:
    using Sensor::Sensor;

    void init();

    void update();

    void stop();

    std::string getJson();

}; // class Sen55

} // namespace Xerxes

#endif // __SEN55__HPP__