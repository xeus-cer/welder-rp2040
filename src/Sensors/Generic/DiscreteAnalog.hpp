#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include "AnalogInput.hpp"

namespace Xerxes
{

namespace ADS1X15 {

enum REGISTERS : uint8_t {
    RESULT = 0x00,
    CONFIG = 0x01,
    LOW_THRESHOLD = 0x02,
    HIGH_THRESHOLD = 0x03
};

enum STATUS : uint16_t {
    BUSY = 0x0000,
    START_SINGLE_NOT_BUSY = 0x8000
};

enum MUX : uint16_t {
    DIFF_0_1 = 0x0000,
    DIFF_0_3 = 0x1000,
    DIFF_1_3 = 0x2000,
    DIFF_2_3 = 0x3000,
    GND_0 = 0x4000,
    GND_1 = 0x5000,
    GND_2 = 0x6000,
    GND_3 = 0x7000
};

enum PGA : uint16_t {
    V_6_144 = 0x0000,
    V_4_096 = 0x0200,
    V_2_048 = 0x0400,
    V_1_024 = 0x0600,
    V_0_512 = 0x0800,
    V_0_256 = 0x0A00
};

enum MODE : uint16_t {
    CONTINUOUS = 0x0000,
    SINGLE = 0x0100
};

enum DR : uint16_t {
    SPS_8 = 0x0000,
    SPS_16 = 0x0020,
    SPS_32 = 0x0040,
    SPS_64 = 0x0060,
    SPS_128 = 0x0080,
    SPS_250 = 0x00A0,
    SPS_475 = 0x00C0,
    SPS_860 = 0x00E0
};

enum COMP_MODE : uint16_t {
    TRADITIONAL = 0x0000,
    WINDOW = 0x0010
};

enum COMP_POLARITY : uint16_t {
    ACTIVE_LOW = 0x0000,
    ACTIVE_HIGH = 0x0008
};

enum COMP_LATCHING : uint16_t {
    NON_LATCHING = 0x0000,
    LATCHING = 0x0004
};

enum COMP_QUEUE : uint16_t {
    ASSERT_AFTER_1 = 0x0000,
    ASSERT_AFTER_2 = 0x0001,
    ASSERT_AFTER_4 = 0x0002,
    DISABLE = 0x0003
};

enum ADDRESS : uint8_t {
    GND = 0x48,
    VDD = 0x49,
    SDA = 0x4A,
    SCL = 0x4B
};

constexpr uint I2C_FREQUENCY_LOW = 400'000;  // 400kHz
constexpr uint I2C_FREQUENCY_HIGH = 3'400'000;  // 3.4MHz


}  // namespace ADS1X15


class DiscreteAnalog : public AnalogInput {
private:
    void _writeConfig(const uint16_t config) const;
    const uint16_t _readConfig() const;
    const int16_t _readConversionResult() const;
    const bool _isBusy() const;
    const bool _isConversionReady() const;

    const int16_t _readChannel(const uint16_t channelRegister) const;

    void _setProgrammableGainAmplifier(const uint16_t ref);
    void _setOperationMode(const uint16_t);
    void _setDataRate(const uint16_t rate);
    void _setAddress(const uint16_t address);

    void _waitForResultUs(const uint32_t timeoutUs) const;
    void _waitForResult() const;

    void _updateFullScaleVoltage();
    void _updateSampleTime();

    uint16_t _programmableGainAmplifier = ADS1X15::PGA::V_4_096;
    uint16_t _operationMode = ADS1X15::MODE::SINGLE;
    uint16_t _dataRate = ADS1X15::DR::SPS_860;
    uint16_t _address = ADS1X15::ADDRESS::GND;
    double _fullScaleVoltage = 4.096;
    uint32_t _sampleTimeUs = 1'000'000 / 860;

protected:
    typedef AnalogInput super;
    constexpr static uint32_t _updateRateHz = 10;  // update frequency in Hz
    constexpr static uint32_t _updateRateUs = _usInS / _updateRateHz;  // update rate in microseconds

public:
    using AnalogInput::AnalogInput;
    using AnalogInput::operator=;
    using AnalogInput::getJsonLast;
    using AnalogInput::getJsonMin;
    using AnalogInput::getJsonMax;
    using AnalogInput::getJsonMean;
    using AnalogInput::getJsonStdDev;

    ~DiscreteAnalog();

    void init();

    void update();

    void stop();

    std::string getJson() override;
};
    
}  // namespace Xerxes