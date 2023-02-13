#include "Sensor.hpp"


namespace Xerxes
{


Sensor::Sensor(Register* reg) : _reg(reg)
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
    }
}

}   // namespace Xerxes