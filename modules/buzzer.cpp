#include "../GeneralHeader.hpp"
#include <pico/time.h>

#define MIN_MS 60000.0f
uint slice_num;


enum scales {
    NO_NOTE =0,
    C_3 = 131,
    C_S_3 = 139,
    D_3 = 147,
    D_S_3 = 156,
    E_3 = 165,
    F_3 = 175,
    F_S_3 = 185,
    G_3 = 196,
    G_S_3 = 208,
    A_3 = 220,
    A_S_3 = 233,
    B_3 = 247,
    C_4 = 262,
    C_S_4 = 277,
    D_4 = 294,
    D_S_4 = 311,
    E_4 = 330,
    F_4 = 349,
    F_S_4 = 370,
    G_4 = 392,
    G_S_4 = 415,
    A_4 = 440,
    A_S_4 = 466,
    B_4 = 494,
    C_5 = 523,
    C_S_5 = 554,
    D_5 = 587,
    D_S_5 = 622,
    E_5 = 659,
    F_5 = 698,
    F_S_5 = 740,
    G_5 = 784,
    G_S_5 = 831,
    A_5 = 880,
    A_S_5 = 932,
    B_5 = 988,
};

enum notes{
    WHOLE_NOTE=1,
    MINIM,
    MINIM_TRIPLET,
    CROCHET,
    CROCHET_TRIPLET,
    QUAVER,
    QUAVER_TRIPLET,
    SEMI_QUAVER,
    SEMI_QUAVER_TRIPLET
};



void hold_note(int bpm, notes note){
    float crochet_length = (float)MIN_MS/bpm;
    float sleep_time_f = 0.0f;
    switch(note){
        case 1:
            sleep_time_f = crochet_length * 4.0f;
            break;
        case 2:
            sleep_time_f = crochet_length * 2.0f;

            break;
        case 3:
            sleep_time_f = crochet_length * 4.0f / 3.0f;
            break;
        case 4:
            sleep_time_f = crochet_length;
            break;
        case 5:
            sleep_time_f = crochet_length * 2.0f / 3.0f;
            break;
        case 6:
            sleep_time_f = crochet_length / 2.0f;
            break;
        case 7:
            sleep_time_f = crochet_length / 3.0f;
            break;
        case 8:
            sleep_time_f = crochet_length / 4.0f;
            break;
        case 9:
            sleep_time_f = crochet_length / 6.0f;
            break;
        default:
            return;
    }
    sleep_ms((int)sleep_time_f);
}

void set_buzzer_freq(uint slice_num, uint gpio, scales note) {
    if (note == 0) {
        pwm_set_gpio_level(gpio, 0); // 消音
        return;
    }

    // 125MHz / freq = div * (wrap + 1)
    // 簡易的にdivを64に固定してwrapを計算
    uint32_t clk_div = 64;
    uint32_t wrap = (125000000 / (clk_div * note)) - 1;

    pwm_set_clkdiv(slice_num, (float)clk_div);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(gpio, wrap / 2); // デューティ比50%
}

void buzzer_init(){
    gpio_set_function(PIN_BUZZER_SIG, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(PIN_BUZZER_SIG);
    pwm_set_enabled(slice_num, true);

}

void daisy_bell(){
    int bpm= 80;
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, G_5);
    sleep_ms(750);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, E_5);
    sleep_ms(750);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_5);
    sleep_ms(750);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, G_4);
    sleep_ms(750);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, A_4);
    sleep_ms(250);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, B_4);
    sleep_ms(250);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_5);
    sleep_ms(250);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, A_4);
    sleep_ms(500);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_5);
    sleep_ms(250);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, G_4);
    sleep_ms(1500);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, NO_NOTE);
}

void misty(){
    int bpm = 80;
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, A_S_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, G_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_4);
    hold_note(bpm,MINIM);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, A_S_3);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_S_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_5);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_5);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, A_S_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_5);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, A_S_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, G_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_S_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_4);
    hold_note(bpm,MINIM);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, NO_NOTE);
}


void sussex(){
    int bpm = 120;
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_4);
    hold_note(bpm,QUAVER);
    hold_note(bpm,SEMI_QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_S_4);
    hold_note(bpm,SEMI_QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_4);
    hold_note(bpm,CROCHET);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, F_4);
    hold_note(bpm,CROCHET);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, G_4);
    hold_note(bpm,MINIM);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_4);
    hold_note(bpm,MINIM);

    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_S_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, F_4);
    hold_note(bpm,QUAVER);
    hold_note(bpm,SEMI_QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, G_4);
    hold_note(bpm,SEMI_QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, A_4);
    hold_note(bpm,CROCHET);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, G_4);
    hold_note(bpm,CROCHET);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, F_4);
    hold_note(bpm,MINIM);
    hold_note(bpm,CROCHET);

    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_4);
    hold_note(bpm,QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_4);
    hold_note(bpm,QUAVER);
    hold_note(bpm,SEMI_QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_S_4);
    hold_note(bpm,SEMI_QUAVER);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, D_4);
    hold_note(bpm,CROCHET);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, E_4);
    hold_note(bpm,CROCHET);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, F_4);
    hold_note(bpm,MINIM);
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, C_4);
    hold_note(bpm,MINIM);

    //end
    set_buzzer_freq(slice_num, PIN_BUZZER_SIG, NO_NOTE);

}


void buzzer_run() {
    daisy_bell();
    misty();
    sussex();

    /*
    while (true) {
        
        // 440Hz (ラ) を1秒鳴らす
        set_buzzer_freq(slice_num, PIN_BUZZER_SIG, 440);
        sleep_ms(1500);

        // 停止
        set_buzzer_freq(slice_num, PIN_BUZZER_SIG, 0);
        sleep_ms(500);
    
        
    }
        */
}