#include "Sensors/Sensirion/Sen55.hpp"

#include "Hardware/Board/xerxes_rp2040.h"
#include "Core/Errors.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/time.h"
#include <array>
#include <sstream>
#include <iostream>
#include "Utils/Log.h"


namespace Xerxes
{


void Sen55::init()
{    
    _devid = DEVID_AIR_POL_CO_NOX_VOC_PM;
    *_reg->desiredCycleTimeUs = 1e5;  // 100ms

    //init i2c with freq 100kHz, return actual frequency
    uint baudrate = i2c_init(i2c0, 100'000);

    xlog_info("Sen55 I2C init, freq: " << baudrate);

    // set the GPIO pin mux to the I2C
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);

    // enable pullups on I2C bus - required if  not powered externally
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // read device product name
    if(_send_command(SenCmd::READ_PRODUCT_NAME))
    {
        uint8_t data[32];
        xlog_debug("Sen55 I2C read product name");
        sleep_ms(50);
        i2c_read_blocking(i2c0, _SEN55_ADDR, data, 32, false);

        // remove every third character (checksums)
        std::string name;
        for (int i = 0; i < 32; i++)
        {
            if (i % 3 != 2)
            {
                name += data[i];
            }
        }
        xlog_info("Sen55 Product name: " << name);
    }


    // write START_MEASUREMENT command to the slave device
    size_t len = _send_command(SenCmd::START_MEASUREMENT);
    if (len == PICO_ERROR_GENERIC)
    {
        // test if device is on the bus:
        int ret;
        uint8_t rxdata;
        ret = i2c_read_blocking(i2c_default, _SEN55_ADDR, &rxdata, 1, false);
        xlog_info("Sen55 I2C read, ret: " << ret << ", rxdata: " << rxdata);
        if (ret < 0)
        {
            xlog_err("Sen55 is not on the bus");
        }
        else
        {
            xlog_err("Sen55 I2C unknown write error");
        }
    }
    else
    {
        xlog_info("Sen55 Measurement started");
        sleep_ms(50);
    }
}

void Sen55::update()
{
// Read the measurement data from the SEN55 sensor
    if(_data_ready())
    {
        uint8_t data[24];
        if(_send_command(SenCmd::READ_MEASURED_VALUES))
        {
            i2c_read_blocking(i2c0, _SEN55_ADDR, data, 24, false);

            // Process the measurement data (example code, adjust as needed)
            uint16_t pm1_0 = (data[0] << 8) | data[1];
            uint16_t pm2_5 = (data[3] << 8) | data[4];
            uint16_t pm4_0 = (data[6] << 8) | data[7];
            uint16_t pm10_0 = (data[9] << 8) | data[10];
            uint16_t rh_pct = (data[12] << 8) | data[13];
            uint16_t temp_c = (data[15] << 8) | data[16];
            uint16_t voc_i = (data[18] << 8) | data[19];
            uint16_t nox_i = (data[21] << 8) | data[22];

            // Print the measurement data (example code, adjust as needed)
            xlog_debug("\n" << \
                "PM1.0: " << pm1_0 << " ug/m3\n" << \
                "PM2.5: " << pm2_5 << " ug/m3\n" << \
                "PM4.0: " << pm4_0 << " ug/m3\n" << \
                "PM10.0: " << pm10_0 << " ug/m3\n" << \
                "RH: " << rh_pct / 100.0 << " %\n" << \
                "Temp: " << temp_c / 100.0 << " C\n" << \
                "VOC: " << voc_i << " ppb\n" << \
                "NOx: " << nox_i << " ppb" \
                );
        }  // if(_send_command(CMD::READ_MEASURED_VALUES))
        else
        {
            xlog_err("Sen55 Not possible to update because of I2C error");
        }
        
    }
    else
    {
        xlog_info("Sen55 data not ready");
    }
}


void Sen55::stop()
{
    
}


std::string Sen55::getJson()
{
    std::stringstream ss;
    ss << "{";
    ss << "}";
    return ss.str();
}


bool Sen55::_data_ready()
{   
    if(_send_command(SenCmd::READ_DATA_READY_FLAG))
    {
        sleep_ms(20);  // wait for the data to be ready (20ms is the minimum)
        uint8_t data[2];  // data[0] - unused, data[1] - data ready flag
        int status = _read_block(data);
        if(status == 2)
        {
            xlog_debug("Sen55 I2C read data: 0x" << std::hex << (int)data[0] << (int)data[1] << std::dec);
            if(data[1] == 0x01 && data[0] == 0x00)
            {
                return true;
            }
        }
        else
        {
            xlog_err("Sen55 I2C read data ready flag error, status: " << status);
        }
    }

    return false;
}


uint8_t Sen55::_crc_8(uint8_t *data)
{
    const uint8_t len = 2;
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x31;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}


int Sen55::_send_command(SenCmd command)
{
    return _send_command((uint16_t)command);
}


int Sen55::_send_command(uint16_t command)
{
    uint8_t cmd[] = {(uint8_t)(command >> 8), (uint8_t)(command & 0xFF)};
    int len = i2c_write_blocking(i2c0, _SEN55_ADDR, cmd, 2, true);
    if(len <= 0)
    {
        xlog_err("Sen55 I2C write error");
    }
    return len;
}


int Sen55::_read_block(uint8_t *two_bytes)
{
    uint8_t data[3];
    i2c_read_blocking(i2c0, _SEN55_ADDR, data, 3, false);
    xlog_debug("Sen55 I2C read data: 0x" << std::hex << (int)data[0] << (int)data[1] << (int)data[2] << std::dec);
    auto crc = _crc_8(data);
    if (crc != data[2])
    {
        xlog_err("Sen55 I2C CRC error");
        return -1;
    }
    else
    {
        two_bytes[0] = data[0];
        two_bytes[1] = data[1];
        return 2;
    }
}

} // namespace Xerxes