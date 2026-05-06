#include "../GeneralHeader.hpp"
#include <hardware/gpio.h>
#include <pico/time.h>

void servo_init(){
    gpio_init(PIN_SERVO_REV);
    gpio_set_function(PIN_SERVO_REV, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PIN_SERVO_REV);

    pwm_config config = pwm_get_default_config();
    // Pico 2 (150MHz) の場合は 150.0f にすると 1カウント = 1us になり、より正確です
    pwm_config_set_clkdiv(&config, 150.0f); 
    pwm_config_set_wrap(&config, 20000); // 20000us = 20ms (50Hz)

    pwm_init(slice_num, &config, true);
}

void set_servo_pulse(float pulse_width_us) {
    pwm_set_gpio_level(PIN_SERVO_REV, (uint16_t)pulse_width_us);
}

void servo_run(int time, servo_dir dir){
        if(dir==CLOCK){
            set_servo_pulse(1420);
        }
        else if(dir==CTRCLOCK){
            set_servo_pulse(1580);    
        }
        sleep_ms(time);
        set_servo_pulse(1500);
    
}