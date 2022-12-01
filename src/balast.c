static uint PWM_0A = 0;
static uint PWM_1B = 3;

void pwm_user_init()
{

    gpio_init(PWM_0A);
    gpio_set_dir(PWM_0A, GPIO_OUT);
    gpio_init(PWM_1B);
    gpio_set_dir(PWM_1B, GPIO_IN);

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
 
    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(0);
    // Set period of 4 cycles (0 to 3 inclusive)
    //pwm_set_wrap(slice_num, 1023);

    pwm_config config = pwm_get_default_config();
    // Set divider, reduces counter clock to sysclock/this value
    //pwm_config_set_clkdiv(&config, 1);
    pwm_config_set_clkdiv_mode(&config, PWM_DIV_FREE_RUNNING);
    //pwm_config_set_wrap(&config, 12);
    // Set the PWM running
    pwm_init(slice_num, &config, true);
    //pwm_set_enabled(slice_num, true);
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1<<15);

    uint slice2 = pwm_gpio_to_slice_num(2);

    pwm_config cfg2 = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg2, PWM_DIV_B_RISING);
    pwm_init(slice2, &cfg2, true);

}