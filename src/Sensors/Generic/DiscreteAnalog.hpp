#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include "AnalogInput.hpp"

namespace Xerxes
{

class DiscreteAnalog : public AnalogInput
{
public:
    using AnalogInput::AnalogInput;
    using AnalogInput::operator=;
    using AnalogInput::getJson;
    using AnalogInput::getJsonLast;
    using AnalogInput::getJsonMin;
    using AnalogInput::getJsonMax;
    using AnalogInput::getJsonMean;
    using AnalogInput::getJsonStdDev;

    ~DiscreteAnalog();

    void init();

    void update();

    void stop();

};
    
}  // namespace Xerxes