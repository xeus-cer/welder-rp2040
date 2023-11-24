#include "Welder.hpp"
#include "Utils/Log.h"
#include "Hardware/Board/xerxes_rp2040.h"
#include "hardware/gpio.h"
#include "Hardware/ClockUtils.hpp"
#include "hardware/pwm.h"
#include <sstream>
#include "Hardware/Sleep.hpp"

namespace Xerxes
{

Welder::~Welder()
{
    stop();
}

void Welder::init()
{
    _devid = DEVID_WELDER;  // device id
    xlog_info("Initializing Welder on PWM5_A/B = GPIO_26/27 = ADC_0/1");

    // enable external 3V3  power supply to welder logic
    gpio_init(EXT_3V3_EN_PIN);
    gpio_set_dir(EXT_3V3_EN_PIN, GPIO_OUT);
    gpio_put(EXT_3V3_EN_PIN, true);

    // map memory to the registers
    pFrequencyHz = _reg->dv0;
    pDurationUs = _reg->dv1;

    /*
    // Tell GPIO 26 and 27 they are allocated to the PWM
    gpio_set_function(PWM5_A_PIN, GPIO_FUNC_PWM);
    gpio_set_function(PWM5_B_PIN, GPIO_FUNC_PWM);
    *pFrequencyHz = 10000.f;  // init with 10kHz

    // Find out which PWM slice is connected to GPIO 26
    slice_num = pwm_gpio_to_slice_num(26);

    pwm_config config = pwm_get_default_config();
    float pwm_clk_div = DEFAULT_SYS_CLOCK_FREQ / pwm_freq;
    xlog_info("PWM clock divider: " << pwm_clk_div << ", pwm frequency: " << pwm_freq);

    pwm_config_set_clkdiv(&config, pwm_clk_div);
    // Load the configuration into our PWM slice, dont start the PWM yet
    pwm_init(slice_num, &config, false);
    */
    // enable external 3V3  power supply to welder logic
    gpio_init(PWM5_A_PIN);
    gpio_set_dir(PWM5_A_PIN, GPIO_OUT);
}

void Welder::update()
{
    /* does not work RN
    // check if the 
    if (*pDurationUs > 0){
        // set the PWM running
        pwm_counter = pwm_freq / *_reg->dv0;
        // Set period of n-cycles
        pwm_set_wrap(slice_num, pwm_counter);

        // Set channel A output high for n/2 cycles
        pwm_set_enabled(slice_num, true);
        pwm_set_gpio_level(PWM5_A_PIN, pwm_counter * .5);  // 50% duty cycle
        pwm_set_gpio_level(PWM5_B_PIN, pwm_counter * .5);  // 50% 
        // wait for the duration
        sleep_us(*pDurationUs);
        // stop the PWM
        pwm_set_gpio_level(PWM5_A_PIN, 0);
        pwm_set_gpio_level(PWM5_B_PIN, 0);
        pwm_set_enabled(slice_num, true);
        // reset the duration
        *pDurationUs = 0;
    }
    */
    gpio_put(USR_LED_PIN, 1);
    gpio_put(PWM5_A_PIN, true);
    sleep_ms(40);
    gpio_put(USR_LED_PIN, 0);
    gpio_put(PWM5_A_PIN, false);
    sleep_hp(1e6);  // 1s
}

std::string Welder::getJson(){
    std::stringstream json;
    json << "{";
    json << "\"frequency\":" << *_reg->dv0 << ",";
    json << "\"duration\":" << *_reg->dv1;
    json << "}";
    return json.str();
};

void Welder::stop()
{
    // disable external 3V3  power supply to welder logic
    gpio_put(EXT_3V3_EN_PIN, false);
}

}  // namespace Xerxes