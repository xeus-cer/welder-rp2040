#include "Core/Register.hpp"


namespace Xerxes
{


Register::Register(/* args */)
{
}


Register::~Register()
{
}


void Register::errorSet(const uint64_t& errorBit)
{
    bitSet(*error, errorBit);
}


void Register::errorClear(const uint64_t& errorBit)
{
    bitClear(*error, errorBit);
}


bool Register::errorCheck(const uint64_t& errorBit)
{
    return (*error & errorBit);
}




} // namespace Xerxes