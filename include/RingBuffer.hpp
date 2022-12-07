#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <array>
#include <random>
#include <cmath>
#include <iostream>

template <class T>
class RingBuffer
{
private:
    uint32_t currentPos {0};
    uint32_t maxSize;
    uint32_t maxCursor {0};
    T* buffer;
public:
    RingBuffer(const uint32_t &maxSize);
    RingBuffer(std::initializer_list<T> il);
    ~RingBuffer();
    void insertOne(T el);

    double getMean();
    double getStdDev();
    double getMax();
    double getMin();
    double getLast();
};


template <class T>
RingBuffer<T>::RingBuffer(std::initializer_list<T> il)
{
    assert(il.size() > 0);
    maxSize = il.size();
    buffer = new T[il.size()]{};
    int i=0;
    for(const auto el : il)
    {
        buffer[i++] = el;
    }
    maxCursor = il.size();
}


template <class T>
RingBuffer<T>::RingBuffer(const uint32_t &maxSize) : maxSize(maxSize), currentPos(0), maxCursor(0)
{   
    assert(maxSize > 0);

    this->buffer = new T[maxSize]{0};
}

template <class T>
RingBuffer<T>::~RingBuffer()
{
    delete buffer;
}


template <class T>
void RingBuffer<T>::insertOne(T el)
{
    if(currentPos >= maxSize)
    {
        currentPos = 0;
    }

    this->buffer[currentPos++] = el;
    if(currentPos > maxCursor) maxCursor = currentPos;
}


template <class T>
double RingBuffer<T>::getStdDev()
{
    double mean = this->getMean();
    double sumOfErrorsSquared(0);
    double StdDev(0);

    uint32_t len = 0;
    for(int i=0; i<maxCursor; i++)
    {
        sumOfErrorsSquared += pow(abs(mean - buffer[i]), 2);
        len++;
    }
    
    StdDev = sqrt(sumOfErrorsSquared / len);

    return StdDev;
}


template <class T>
double RingBuffer<T>::getMean()
{
    double mean(0);
    double sumOfElements(0);

    uint32_t len = 0;
    for(int i=0; i<maxCursor; i++)
    {
        sumOfElements += buffer[i];
        len++;
    }
    
    mean = sumOfElements / len;
    
    return mean;
}


template <class T>
double RingBuffer<T>::getMin()
{
    double min = INFINITY;
    for(int i=0; i<maxCursor; i++)
    {
        if(buffer[i] < min) min=buffer[i];
    }
    
    return min;
}


template <class T>
double RingBuffer<T>::getMax()
{
    double max = -INFINITY;
    for(int i=0; i<maxCursor; i++)
    {
        if(buffer[i] > max) max=buffer[i];
    }
    
    return max;
}

template <class T>
double RingBuffer<T>::getLast()
{
    double last = buffer[currentPos - 1];
    return last;
}


#endif // RINGBUFFER_HPP