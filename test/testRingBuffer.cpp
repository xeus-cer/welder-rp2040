#include <gtest/gtest.h>
#include <iostream>
#include "../include/RingBuffer.hpp"


TEST(RingBuffer, getStdDevDouble){
    RingBuffer<double> rb(10);
    for(int i=0; i<100; i++)
    {
        rb.insertOne(i/10.0);
    }
    EXPECT_DOUBLE_EQ(rb.getStdDev(), 0.28722813232690143);
}

TEST(RingBuffer, getStdDevInt){
    RingBuffer<int> rb(10);
    for(int i=0; i<100; i++)
    {
        rb.insertOne(i);
    }
    EXPECT_DOUBLE_EQ(rb.getStdDev(), 2.8722813232690143);
}

TEST(RingBuffer, getMean){ 
    RingBuffer<double> rb(10);
    for(int i=1; i<100; i++)
    {
        rb.insertOne(i);
    }
    EXPECT_DOUBLE_EQ(rb.getMean(), 94.5);
}

TEST(RingBuffer, getMinDouble){ 
    RingBuffer<double> rb {1, 2, -3.14, 4, 5, INFINITY};

    EXPECT_DOUBLE_EQ(rb.getMin(), 1);
}

TEST(RingBuffer, getMaxDouble){ 
    RingBuffer<double> rb {-6, 9, 2, 3.14, 4, 5, -INFINITY};

    EXPECT_DOUBLE_EQ(rb.getMax(), 9);
}

TEST(RingBuffer, getLast){ 
    RingBuffer<double> rb(10);
    for(int i=1; i<55; i++)
    {
        rb.insertOne(i);
    }
    EXPECT_DOUBLE_EQ(rb.getLast(), 54);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}