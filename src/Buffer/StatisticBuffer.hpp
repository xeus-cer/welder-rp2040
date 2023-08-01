#ifndef STATISTIC_BUFFER_HPP
#define STATISTIC_BUFFER_HPP

#include "RingBuffer.hpp"

namespace Xerxes
{


template <class T>
class StatisticBuffer : public RingBuffer<T>
{
protected:
    float min;
    float max;
    float mean;
    float stdDev;
    float median;
public:
    using RingBuffer<T>::RingBuffer; // inherit constructors
    void updateStatistics();

    const float & getMean();
    const float & getStdDev();
    const float & getMax();
    const float & getMin();
    const float & getMedian();
    void getStatistics(T* min, T* max, T* mean, T* stdDev);
};


template <class T>
void StatisticBuffer<T>::getStatistics(T* min, T* max, T* mean, T* stdDev)
{
    *min = this->min;
    *max = this->max;
    *mean = this->mean;
    *stdDev = this->stdDev;
}


template <class T>
void StatisticBuffer<T>::updateStatistics()
{
    mean = 0;
    min = INFINITY;
    max = -INFINITY;

    double sumOfElements {0};
    double sumOfSquaredErrors {0};

    for(int i=0; i<this->maxCursor; i++)
    {
        T el = this->buffer[i];
        sumOfElements += el;

        if(el < min)
        {
            min = el;
        }

        if(el > max)
        {
            max = el;
        }
    }

    mean = sumOfElements / this->maxCursor;

    for(int i=0; i<this->maxCursor; i++)
    {
        sumOfSquaredErrors += powf(this->buffer[i] - mean, 2);
    }

    stdDev = sqrtf(sumOfSquaredErrors / this->maxCursor);

    // Calculate the median
    std::sort(sortedBuffer.begin(), sortedBuffer.end());
    if (this->maxCursor % 2 == 0)
    {
        // If the number of elements is even, take the average of the middle two elements
        median = (sortedBuffer[this->maxCursor / 2 - 1] + sortedBuffer[this->maxCursor / 2]) / 2.0;
    }
    else
    {
        // If the number of elements is odd, take the middle element
        median = sortedBuffer[this->maxCursor / 2];
    }
}


template <class T>
const float & StatisticBuffer<T>::getStdDev()
{
    return stdDev;
}


template <class T>
const float & StatisticBuffer<T>::getMean()
{
    return mean;
}


template <class T>
const float & StatisticBuffer<T>::getMin()
{
    return this->min;
}


template <class T>
const float & StatisticBuffer<T>::getMax()
{
    return max;
}

template <class T>
const float & StatisticBuffer<T>::getMedian()
{
    return median;
}


} //namespace Xerxes

#endif // !STATISTIC_BUFFER_HPP