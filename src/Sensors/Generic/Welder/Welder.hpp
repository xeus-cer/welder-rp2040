#ifndef __WELDER
#define __WELDER

#include "Sensors/Generic/DIO/4DI4DO.hpp"
#include "Core/Register.hpp"

namespace Xerxes
{

    class Welder : public _4DI4DO
    {
    private:
        // sensor specific variables
        uint8_t numChannels = 4; // number of channels, default is 4

        constexpr static uint32_t _updateRateHz = 100;                    // update frequency in Hz
        constexpr static uint32_t _updateRateUs = _usInS / _updateRateHz; // update rate in microseconds

        uint32_t *pFrequencyHz; // = _reg->pv0;
        uint32_t *pDurationUs;  // = _reg->pv1;

        uint slice_num;       // which slice to use for PWM operation
        uint16_t pwm_counter; // counter for PWM operation
        // Set divider, reduces counter clock to sysclock/this value
        const static uint32_t pwm_freq = 1000000; // PWM frequency in Hz

        typedef _4DI4DO super;

    protected:
        Register *_reg;

    public:
        Welder(Register *reg) : _reg(reg){};

        Welder() : _reg(nullptr){};

        // sensor specific functions
        ~Welder();

        void init();
        void update();
        void stop();
        std::string getJson() override;
        std::string getInfoJson() const override;
    }; // class Welder

} // namespace Xerxes

#endif // __WELDER