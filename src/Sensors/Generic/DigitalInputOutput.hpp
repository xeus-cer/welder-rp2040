#ifndef __DIGITALINPUTOUTPUT_HPP
#define __DIGITALINPUTOUTPUT_HPP

#include "Sensors/Peripheral.hpp"
#include "Hardware/xerxes_rp2040.h"


namespace Xerxes
{


class DigitalInputOutput : public Peripheral
{
protected:
    /** Digital values */
    uint32_t* dv0;
    uint32_t* dv1;
    uint32_t* dv2;
    uint32_t* dv3;

    uint32_t used_iomask = SHIELD_MASK;

public:

    /**
     * @brief Construct a new Digital Input Output object
     */
    DigitalInputOutput() :
        dv0(nullptr), dv1(nullptr), dv2(nullptr), dv3(nullptr)
    {};

    /**
     * @brief Construct a new Digital Input Output object, with pointers to the digital values
     * 
     * @param dv0 - pointer to the digital value 0
     * @param dv1 - pointer to the digital value 1
     * @param dv2 - pointer to the digital value 2
     * @param dv3 - pointer to the digital value 3
     */
    DigitalInputOutput(uint32_t *dv0, uint32_t *dv1, uint32_t *dv2, uint32_t *dv3) :
        dv0(dv0), dv1(dv1), dv2(dv2), dv3(dv3)
    {};
    
    /**
     * @brief Destroy the Digital Input Output object
     */
    ~DigitalInputOutput();

    /**
     * @brief Initialize the DIO
     * 
     * @param iomask - 32 bit mask of the DIO pins, 0 = do not change direction, 1 = change direction
     * @param direction - 32 bit mask of the DIO pins, 0 = input, 1 = output
     * 
     */
    void init(uint32_t iomask, uint32_t direction);


    /**
     * @brief Update the DIO
     * 
     */
    void update();


    /**
     * @brief Stop the DIO
     * 
     */
    void stop();
};


}   // namespace Xerxes


#endif // !__DIGITALINPUTOUTPUT_HPP