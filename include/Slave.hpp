#ifndef __SLAVE_HPP
#define __SLAVE_HPP

#include "Protocol.hpp"

namespace Xerxes
{


class Slave
{
private:
    Protocol *xp;
public:
    Slave(Protocol *protocol);
    ~Slave();

};

Slave::Slave(Protocol *protocol) : xp(protocol)
{
}

Slave::~Slave()
{
}
    

} // namespace Xerxes

#endif // !__SLAVE_HPP