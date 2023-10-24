#include "DiscreteAnalog.hpp"
#include "Hardware/Board/xerxes_rp2040.h"
#include "hardware/adc.h"
#include <string>
#include <sstream>
#include "pico/time.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include <bitset>

namespace Xerxes
{

DiscreteAnalog::~DiscreteAnalog()
{
    stop();
}

void DiscreteAnalog::_writeConfig(const uint16_t config) const {
    uint8_t data[3];
    data[0] = ADS1X15::REGISTERS::CONFIG;  // points to config register
    // MSB first
    data[1] = (config >> 8) & 0xFF;
    data[2] = config & 0xFF;
    xlog_debug("ADS1115 config write: " << config << " (" << std::bitset<16>(config) << ")");
    int len = i2c_write_blocking(PICO_DEFAULT_I2C, _address, data, 3, false);
    assert(len == 3);  // check if write was successful
}

const uint16_t DiscreteAnalog::_readConfig() const {
    uint8_t data[1] = {ADS1X15::REGISTERS::CONFIG};  // points to config register
    int len = i2c_write_blocking(PICO_DEFAULT_I2C, _address, data, 1, false);
    assert(len == 1);  // check if write was successful

    uint8_t result[2];
    // read result, MSB first
    len = i2c_read_blocking(PICO_DEFAULT_I2C, _address, result, 2, false);
    assert(len == 2);  // check if read was successful

    // convert to uint16_t
    const uint16_t resultInt = (result[0] << 8) | result[1];
    xlog_debug("ADS1115 config read: " << resultInt << " (" << std::bitset<16>(resultInt) << ")");
    return resultInt;
}

const int16_t DiscreteAnalog::_readConversionResult() const {
    uint8_t data[1] = {ADS1X15::REGISTERS::RESULT};  // points to conversion register
    int len = i2c_write_blocking(PICO_DEFAULT_I2C, _address, data, 1, false);
    assert(len == 1);  // check if write was successful

    uint8_t result[2];
    // read result, MSB first
    len = i2c_read_blocking(PICO_DEFAULT_I2C, _address, result, 2, false);
    assert(len == 2);  // check if read was successful

    // convert to int16_t
    int16_t resultInt = (result[0] << 8) | result[1];
    xlog_debug("ADS1115 result read: " << resultInt);
    return resultInt;
}

const bool DiscreteAnalog::_isBusy() const {
    auto config = _readConfig();
    const bool busy = (config & ADS1X15::STATUS::START_SINGLE_NOT_BUSY) == ADS1X15::STATUS::BUSY;
    xlog_debug("ADS1115 busy: " << busy);
    return busy;
}

const bool DiscreteAnalog::_isConversionReady() const {
    auto config = _readConfig();
    const bool ready = (config & ADS1X15::STATUS::START_SINGLE_NOT_BUSY) == ADS1X15::STATUS::START_SINGLE_NOT_BUSY;
    xlog_debug("ADS1115 conversion ready: " << ready);
    return ready;
}

void DiscreteAnalog::_waitForResultUs(const uint32_t timeoutUs) const {
    uint32_t start = time_us_32();
    while (!_isConversionReady()) {
        if (time_us_32() - start > timeoutUs) {
            xlog_error("ADS1115 conversion timeout");
            throw std::runtime_error("ADS1115 conversion timeout");
        }
    }
}

void DiscreteAnalog::_waitForResult() const {
    // wait for conversion + 2ms headroom for communication
    xlog_debug("ADS1115 sample time: " << _sampleTimeUs << "us");
    _waitForResultUs(_sampleTimeUs + 2000);
}

void DiscreteAnalog::_updateFullScaleVoltage() {
    switch (_programmableGainAmplifier) {
        case ADS1X15::PGA::V_6_144:
            _fullScaleVoltage = 6.144;
            break;
        case ADS1X15::PGA::V_4_096:
            _fullScaleVoltage = 4.096;
            break;
        case ADS1X15::PGA::V_2_048:
            _fullScaleVoltage = 2.048;
            break;
        case ADS1X15::PGA::V_1_024:
            _fullScaleVoltage = 1.024;
            break;
        case ADS1X15::PGA::V_0_512:
            _fullScaleVoltage = 0.512;
            break;
        case ADS1X15::PGA::V_0_256:
            _fullScaleVoltage = 0.256;
            break;
        default:
            xlog_warning("ADS1115 invalid PGA: " << _programmableGainAmplifier);
            xlog_warning("ADS1115 falling back to 4.096V");
            _fullScaleVoltage = 4.096;
    }
    xlog_debug("ADS1115 full scale voltage set to: " << _fullScaleVoltage);
}

void DiscreteAnalog::_updateSampleTime() {
    float sampleFreq;
    switch(_dataRate) {
        case ADS1X15::DR::SPS_8:
            sampleFreq = 8;
            break;
        case ADS1X15::DR::SPS_16:
            sampleFreq = 16;
            break;
        case ADS1X15::DR::SPS_32:
            sampleFreq = 32;
            break;
        case ADS1X15::DR::SPS_64:
            sampleFreq = 64;
            break;
        case ADS1X15::DR::SPS_128:
            sampleFreq = 128;
            break;
        case ADS1X15::DR::SPS_250:
            sampleFreq = 250;
            break;
        case ADS1X15::DR::SPS_475:
            sampleFreq = 475;
            break;
        case ADS1X15::DR::SPS_860:
            sampleFreq = 860;
            break;
        default:
            xlog_warning("ADS1115 invalid data rate: " << _dataRate);
            xlog_warning("ADS1115 falling back to 64 SPS");
            sampleFreq = 64;        
    }
    _sampleTimeUs = (1'000'000.0f / sampleFreq);
    xlog_debug("ADS1115 sample time set to: " << _sampleTimeUs << "us");
}

const int16_t DiscreteAnalog::_readChannel(const uint16_t channel_register) const{
    const uint16_t config = _operationMode |
                            _programmableGainAmplifier |
                            _dataRate |
                            channel_register |
                            ADS1X15::COMP_QUEUE::DISABLE |
                            ADS1X15::STATUS::START_SINGLE_NOT_BUSY;
    _writeConfig(config);
    _waitForResult();
    return _readConversionResult();
}

void DiscreteAnalog::_setProgrammableGainAmplifier(const uint16_t ref) {
    // sanitize input
    if (
        ref != ADS1X15::PGA::V_6_144 &&
        ref != ADS1X15::PGA::V_4_096 &&
        ref != ADS1X15::PGA::V_2_048 &&
        ref != ADS1X15::PGA::V_1_024 &&
        ref != ADS1X15::PGA::V_0_512 &&
        ref != ADS1X15::PGA::V_0_256
    ) {
        xlog_warning("ADS1115 invalid PGA: " << ref);
        xlog_warning("ADS1115 falling back to 4.096V");
        _programmableGainAmplifier = ADS1X15::PGA::V_4_096;
    }
    _programmableGainAmplifier = ref;
    xlog_debug("ADS1115 PGA set to: " << ref);
    _updateFullScaleVoltage();
}

void DiscreteAnalog::_setOperationMode(const uint16_t mode) {
    if(
        mode != ADS1X15::MODE::CONTINUOUS &&
        mode != ADS1X15::MODE::SINGLE
    ) {
        xlog_warning("ADS1115 invalid operation mode: " << mode);
        xlog_warning("ADS1115 falling back to single mode");
        _operationMode = ADS1X15::MODE::SINGLE;
    }
    _operationMode = mode;
    xlog_debug("ADS1115 operation mode set to: " << mode);
}

void DiscreteAnalog::_setDataRate(const uint16_t rate) {
    // sanitize input
    if(
        rate != ADS1X15::DR::SPS_8 &&
        rate != ADS1X15::DR::SPS_16 &&
        rate != ADS1X15::DR::SPS_32 &&
        rate != ADS1X15::DR::SPS_64 &&
        rate != ADS1X15::DR::SPS_128 &&
        rate != ADS1X15::DR::SPS_250 &&
        rate != ADS1X15::DR::SPS_475 &&
        rate != ADS1X15::DR::SPS_860
    ) {
        xlog_warning("ADS1115 invalid data rate: " << rate);
        xlog_warning("ADS1115 falling back to 8 SPS");
        _dataRate = ADS1X15::DR::SPS_64;
    }

    _dataRate = rate;
    xlog_debug("ADS1115 data rate set to: " << rate);
    _updateSampleTime();
}

void DiscreteAnalog::_setAddress(const uint16_t address) {
    _address = address;
}

void DiscreteAnalog::init()
{
    _devid = DEVID_IO_3AI;
    _label = "3AI 16bit ADS1115, VRef 2.5V";

    // enable power supply to sensor
    gpio_init(ADC0_PIN);
    gpio_set_dir(ADC0_PIN, GPIO_OUT);

    // enable VREF for sensor
    gpio_put(ADC0_PIN, true);

    // initialize I2C
    i2c_init(PICO_DEFAULT_I2C, ADS1X15::I2C_FREQUENCY_HIGH);

    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);

    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(I2C0_SDA_PIN, I2C0_SCL_PIN, GPIO_FUNC_I2C));

    xlog_info("I2C initialized");

    // read 1 bytes from I2C to check if it works
    uint8_t data[1];
    int len = i2c_read_blocking(PICO_DEFAULT_I2C, _address, data, 1, false);
    if(len == PICO_ERROR_GENERIC) {
        xlog_error("I2C read error, device not ready");
        throw std::runtime_error("I2C read error, device not ready");
    }
    xlog_info("I2C read success, device ready for use");

    _setDataRate(ADS1X15::DR::SPS_475);
    _setProgrammableGainAmplifier(ADS1X15::PGA::V_4_096);
    _setOperationMode(ADS1X15::MODE::SINGLE);

    // set update rate
    *_reg->desiredCycleTimeUs = _updateRateUs;

    // initialize ringbuffer with size of RING_BUFFER_LEN (defined in Definitions.h)
    rbpv0 = StatisticBuffer<float>(_updateRateHz);
    rbpv1 = StatisticBuffer<float>(_updateRateHz);
    rbpv2 = StatisticBuffer<float>(_updateRateHz);
    rbpv3 = StatisticBuffer<float>(_updateRateHz);
}

void DiscreteAnalog::update() {
    sleep_us(2000); // wait for sensor to power up

    double valChannel1 = 0;
    double valChannel2 = 0;
    double valChannel3 = 0;
    double valReferenceVoltage = 1;
    try {
        valChannel1 = _readChannel(ADS1X15::MUX::GND_0) * _fullScaleVoltage / 32768;  
        valChannel2 = _readChannel(ADS1X15::MUX::GND_1) * _fullScaleVoltage / 32768;
        valChannel3 = _readChannel(ADS1X15::MUX::GND_2) * _fullScaleVoltage / 32768;
        valReferenceVoltage = _readChannel(ADS1X15::MUX::GND_3) * _fullScaleVoltage / 32768;
        // 32768 = 2^15 in both directions
    } catch (const std::exception& e) {
        xlog_error("ADS1115 read error: " << e.what());
    }

    *_reg->pv0 = valChannel1 / valReferenceVoltage;
    *_reg->pv1 = valChannel2 / valReferenceVoltage;
    *_reg->pv2 = valChannel3 / valReferenceVoltage;
    *_reg->pv3 = valReferenceVoltage;

    this->Sensor::update();
}

void DiscreteAnalog::stop()
{
    // disable power supply to sensor
    gpio_put(ADC0_PIN, false);
    gpio_set_dir(ADC0_PIN, GPIO_IN);
}


std::string DiscreteAnalog::getJson()
{
    using namespace std;
    stringstream ss;

    ss << endl << "{" << endl;
    ss << "  \"Last\":" << getJsonLast() << "," << endl;
    ss << "  \"Min\":" << getJsonMin() << "," << endl;
    ss << "  \"Max\":" << getJsonMax() << "," << endl;
    ss << "  \"Mean\":" << getJsonMean() << "," << endl;
    ss << "  \"StdDev\":" << getJsonStdDev() << endl;
    ss << "}";

    return ss.str();
}

}  // namespace Xerxes