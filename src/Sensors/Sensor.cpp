#include "Sensor.hpp"
#include <bitset>

namespace Xerxes
{

    Sensor::Sensor(Register *reg) : _reg(reg)
    {
        // initialize ringbuffer with size of RING_BUFFER_LEN (defined in Definitions.h)
        rbpv0 = StatisticBuffer<float>(RING_BUFFER_LEN);
        rbpv1 = StatisticBuffer<float>(RING_BUFFER_LEN);
        rbpv2 = StatisticBuffer<float>(RING_BUFFER_LEN);
        rbpv3 = StatisticBuffer<float>(RING_BUFFER_LEN);
    }

    void Sensor::update()
    {
        // if calcStat is true, update statistics
        if (_reg->config->bits.calcStat)
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
        }
    }

    std::string Sensor::getInfoJson() const
    {
        auto gain0 = (float)*_reg->gainPv0;
        auto gain1 = (float)*_reg->gainPv1;
        auto gain2 = (float)*_reg->gainPv2;
        auto gain3 = (float)*_reg->gainPv3;
        auto offset0 = (float)*_reg->offsetPv0;
        auto offset1 = (float)*_reg->offsetPv1;
        auto offset2 = (float)*_reg->offsetPv2;
        auto offset3 = (float)*_reg->offsetPv3;
        uint64_t uuid = *_reg->uid;
        std::stringstream ss;
        ss << "{" << std::endl;
        ss << "  \"Address\": " << (int)*_reg->devAddress << "," << std::endl;
        ss << "  \"ID\": " << (int)_devid << "," << std::endl;
        // ss << "Type: " << typeid(this).name() << "," << std::endl;  // needs to enable rrti in cmake
        ss << "  \"Label\": \"" << _label << "\"," << std::endl;
        ss << "  \"UUID\": \"" << uuid << "\"," << std::endl;
        ss << "  \"Version\": \"" << __VERSION << "\"," << std::endl;
        ss << "  \"Build date\": \"" << __DATE__ << "\" " << std::endl;
        // ss << "  \"Errors\": 0b" << std::bitset<32>(*_reg->error) << "," << std::endl;
        // ss << "  \"Status\": " << (int)*_reg->status << "," << std::endl;
        // ss << "  \"Update rate\": " << (int)*_reg->desiredCycleTimeUs << "," << std::endl;
        // ss << "  \"Gains\": [" << gain0 << ", " << gain1 << ", " << gain2 << ", " << gain3 << "]," << std::endl;
        // ss << "  \"Offsets\": [" << offset0 << ", " << offset1 << ", " << offset2 << ", " << offset3 << "]" << std::endl;
        ss << "}" << std::endl;

        return ss.str();
    }

} // namespace Xerxes