#include <gtest/gtest.h>

#include "Message.hpp"
#include "MessageId.h"

using namespace std;


TEST(Message, toPacket)
{   
    std::vector<uint8_t> payload {};
    Xerxes::Message m(1, 2, MSGID_ACK_OK, payload);

    Xerxes::Packet p = m.toPacket();
    
    EXPECT_EQ(p.size(), 7);

    EXPECT_EQ(p.at(0), SOH);
    EXPECT_EQ(p.at(1), 7);
    EXPECT_EQ(p.at(2), 1);
    EXPECT_EQ(p.at(3), 2);
    EXPECT_EQ(p.at(4), 2);
    EXPECT_EQ(p.at(5), 0);
    EXPECT_EQ(p.at(6), 243);
}


TEST(Packet, EmptyPacketGenerator)
{
    Xerxes::Packet p = Xerxes::Packet::EmptyPacket();
    
    EXPECT_EQ(p.size(), 3);
    EXPECT_EQ(p.at(0), SOH);
    EXPECT_EQ(p.at(1), 3);
    EXPECT_EQ(p.at(2), 0xFC);
}


TEST(Message, PingPacket)
{
    Xerxes::Message pingMsg(0x1E, 0xBA, MSGID_PING);
    Xerxes::Packet p = pingMsg.toPacket();
    
    EXPECT_EQ(p.size(), 7);
    EXPECT_EQ(p.at(0), SOH);
    EXPECT_EQ(p.at(1), 7);
    EXPECT_EQ(p.at(2), 0x1E);
    EXPECT_EQ(p.at(3), 0xba);
    EXPECT_EQ(p.at(4), 0);
    EXPECT_EQ(p.at(5), 0);
    EXPECT_EQ(p.at(6), 0x20);
}

TEST(Message, MessageFromPacket)
{
    Xerxes::Message pingMsg(0x1E, 0xBA, MSGID_READ);
    Xerxes::Packet p = pingMsg.toPacket();
    Xerxes::Message pingMsg2(&p);
    Xerxes::Packet p2 = pingMsg2.toPacket();
    
    EXPECT_EQ(p2.size(), 7);
    EXPECT_EQ(p2.at(0), SOH);
    EXPECT_EQ(p2.at(1), 7);
    EXPECT_EQ(p2.at(2), 0x1E);
    EXPECT_EQ(p2.at(3), 0xba);
    EXPECT_EQ(p2.at(4), 1);
    EXPECT_EQ(p2.at(5), 2);
    EXPECT_EQ(p2.at(6), 0x1D);
}