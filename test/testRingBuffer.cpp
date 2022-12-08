#include <gtest/gtest.h>
#include <iostream>
#include "../include/StatisticBuffer.hpp"


TEST(StatisticBuffer, getStdDevDouble)
{
    Xerxes::StatisticBuffer<double> rb(10);
    for(int i=0; i<100; i++)
    {
        rb.insertOne(i/10.0);
    }
    rb.updateStatistics();
    EXPECT_FLOAT_EQ(rb.getStdDev(), 0.28722813232690143);
}


TEST(StatisticBuffer, getStdDevInt)
{
    Xerxes::StatisticBuffer<int> rb(10);
    for(int i=0; i<100; i++)
    {
        rb.insertOne(i);
    }
    rb.updateStatistics();
    EXPECT_FLOAT_EQ(rb.getStdDev(), 2.8722813232690143);
}


TEST(StatisticBuffer, getMean)
{ 
    Xerxes::StatisticBuffer<double> rb(10);
    for(int i=1; i<100; i++)
    {
        rb.insertOne(i);
    }
    rb.updateStatistics();
    EXPECT_FLOAT_EQ(rb.getMean(), 94.5);
}


TEST(StatisticBuffer, getMinDouble)
{ 
    Xerxes::StatisticBuffer<double> rb {1, 2, -3.14, 4, 5, INFINITY};
    rb.updateStatistics();

    EXPECT_FLOAT_EQ(rb.getMin(), -3.14);
}


TEST(StatisticBuffer, getMaxDouble)
{ 
    Xerxes::StatisticBuffer<double> rb {-6, 9, 2, 3.14, 4, 5, -INFINITY};
    rb.updateStatistics();

    EXPECT_FLOAT_EQ(rb.getMax(), 9);
}


TEST(RingBuffer, getLast)
{ 
    Xerxes::RingBuffer<double> rb(10);
    for(int i=1; i<55; i++)
    {
        rb.insertOne(i);
    }
    double lastElement = rb.getLast();
    EXPECT_FLOAT_EQ(lastElement, 54);
}


TEST(StatisticBuffer, initializerList)
{
    Xerxes::StatisticBuffer<float> rb {-5, 1, 3.14, -INFINITY, 9};

    EXPECT_FLOAT_EQ(rb.getMin(), -INFINITY);
    EXPECT_FLOAT_EQ(rb.getLast(), 9);
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}