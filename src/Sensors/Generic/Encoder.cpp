#include "Encoder.hpp"

#include "Hardware/Board/xerxes_rp2040.h"
#include "hardware/gpio.h"
#include <sstream>
#include <iostream>
#include "pico/time.h"

namespace Xerxes
{


void Encoder::init()
{
    // Initialize the DIO pins for the 4DI4DO shield (GPIO0-5 | GPIO8-9)
    super::init();
    *_reg->sv0 = 0;
    *_reg->sv1 = 0;
    *_reg->sv2 = 0;
    *_reg->sv3 = 0;
    encoderVal = _reg->sv0;

    _devid = DEVID_ENC_1000PPR;
}


void Encoder::encoderIrqHandler(uint gpio)
{
    uint snap = gpio_get_all();
    // read the encoder pin B
    bool pinA = snap & (1 << ENCODER_PIN_A);
    bool pinB = snap & (1 << ENCODER_PIN_B);
    bool pinZ = snap & (1 << ENCODER_PIN_Z);

    if(pinA)
    {
        // if pin B is high, then the encoder is rotating clockwise
        if(pinB)
        {
            // increment the encoder count
            (*encoderVal) ++;

        }
        else
        {
            // decrement the encoder count
            (*encoderVal) --;
        } 
    }
    if(pinZ)
    {
        int8_t _add = 0;
        if((*encoderVal) % 1000 >= 500 && (*encoderVal) > 0) 
        {
            _add = 1;
        }
        if((*encoderVal) % 1000 < -500 && (*encoderVal) < 0)
        {
            _add = -1;
        }
        {
            (*encoderVal) = (_add + int((*encoderVal) / 1000)) * 1000;
        }
    }
}


std::string Encoder::getJson()
{
    using namespace std;
    stringstream ss;

    ss << "{" << endl;
    ss << "\t\"counter\": " << (*encoderVal) << endl;
    ss << "}" << endl;

    return ss.str();
}




void Encoder::update()
{
    // do nothing
}

} // namespace Xerxes