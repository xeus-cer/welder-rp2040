#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <array>
#include <random>
#include <cmath>
#include <iostream>

namespace Xerxes
{


/**
 * @brief Ring buffer class 
 * 
 * This class implements a ring buffer. It is used to store the last n elements
 * of a stream of data. The buffer is implemented as a circular buffer.
 * 
 * @tparam T - type of the elements to be stored in the buffer
 */
template <class T>
class RingBuffer
{
private:
    uint32_t currentPos {0};
    uint32_t maxSize;
protected:
    uint32_t maxCursor {0};
    T* buffer;
    bool saturated {false};

public:
    RingBuffer();
    RingBuffer(const uint32_t &maxSize);
    RingBuffer(std::initializer_list<T> il);
    ~RingBuffer();

    void insertOne(const T el);
    const T & getLast();
};


template <class T>
RingBuffer<T>::RingBuffer()
{
}


template <class T>
RingBuffer<T>::RingBuffer(std::initializer_list<T> il) : currentPos(0)
{
    size_t size = il.size();
    maxSize = size;
    buffer = new T[size]{};
    int i = 0;
    for(const auto el : il)
    {
        buffer[i++] = el;
    }
    maxCursor = size;
    saturated = true;
}


template <class T>
RingBuffer<T>::RingBuffer(const uint32_t &maxSize) : maxSize(maxSize), currentPos(0), maxCursor(0)
{   
    this->buffer = new T[maxSize]{0};
}

template <class T>
RingBuffer<T>::~RingBuffer()
{
    // delete buffer; // TODO: uncomment this line !!!
}


template <class T>
void RingBuffer<T>::insertOne(const T el)
{
    if(currentPos >= maxSize)
    {
        currentPos = 0;
        saturated = true;
    }

    this->buffer[currentPos++] = el;
    if(currentPos > maxCursor) maxCursor = currentPos;
}


template <class T>
const T & RingBuffer<T>::getLast()
{
    if(this->currentPos > 0)
    {
        return this->buffer[this->currentPos - 1];
    }
    else
    {
        return this->buffer[this->maxCursor - 1];
    }
}

    
} // namespace Xerxes

#endif // RINGBUFFER_HPP