#include "../GeneralHeader.hpp"

#define DATA_INVALID -32768

lora_payload_t lora_data;

sx126x_pico_context_t ctx = {
    .spi = spi0,
    .cs_pin = PIN_LAMBDA_CS,
    .busy_pin = PIN_LAMBDA_BUSY,
    .reset_pin = PIN_LAMBDA_RESET
};



bool dio1_flag = false;

uint8_t tx_buf[] = "HELLO 868";
uint8_t tx_len = strlen((char*)tx_buf); 

void dio1_irq_handler(uint gpio, uint32_t events) {
    dio1_flag = true;
}

static inline void sx_wait_busy()
{
    while (gpio_get(PIN_LAMBDA_BUSY)) {
        tight_loop_contents();
    }
}

void lora_init() {

    gpio_set_irq_enabled_with_callback(
        PIN_LAMBDA_DIO1,
        GPIO_IRQ_EDGE_RISE,
        true,
        &dio1_irq_handler
    );

    sx126x_reset(&ctx);

    sleep_ms(10);
    
    sx126x_wakeup(&ctx);

    sleep_ms(10);


    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_RC);
    sx126x_set_reg_mode(&ctx, SX126X_REG_MODE_LDO);  
    sx126x_cal(&ctx, SX126X_CAL_ALL);  


    sx126x_set_pkt_type(&ctx, SX126X_PKT_TYPE_LORA);


    sx126x_pa_cfg_params_t pa = {
        .pa_duty_cycle = 0x04,
        .hp_max = 0x07,
        .device_sel = 0x00,
        .pa_lut = 0x01
    };


    sx126x_set_rf_freq(&ctx, 868000000);

    sx126x_set_pa_cfg(&ctx, &pa);


    sx126x_set_tx_params(&ctx, 11, SX126X_RAMP_200_US);


    sx126x_mod_params_lora_t mod = {
        .sf = SX126X_LORA_SF7,
        .bw = SX126X_LORA_BW_125,
        .cr = SX126X_LORA_CR_4_5,
        .ldro = 0
    };
    sx126x_set_lora_mod_params(&ctx, &mod);

    sx126x_pkt_params_lora_t pkt = {
        .preamble_len_in_symb = 16,
        .header_type = SX126X_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes = 17,
        .crc_is_on = true,
        .invert_iq_is_on = false
    };
    sx126x_set_lora_pkt_params(&ctx, &pkt);



    sx126x_set_buffer_base_address(&ctx, 0x00, 0x80);

    sx126x_set_dio_irq_params(
        &ctx,
        SX126X_IRQ_TX_DONE | SX126X_IRQ_TIMEOUT,
        SX126X_IRQ_TX_DONE,
        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );

    sx126x_set_lora_sync_word(&ctx, 0x34);


    gpio_put(PIN_LAMBDA_RX, 0);
    gpio_put(PIN_LAMBDA_TX, 1);
    sleep_ms(5);
}


void lora_send() {

    while (gpio_get(PIN_LAMBDA_BUSY));
    sleep_ms(1);

    sx126x_write_buffer(&ctx, 0x00, (uint8_t *)&lora_data, sizeof(lora_data));

    // ★重要：ここ追加
    sx126x_get_status(&ctx, NULL);
    sleep_us(200);

    while (gpio_get(PIN_LAMBDA_BUSY));
    sleep_ms(1);

    gpio_put(PIN_LAMBDA_TX, 1);
    gpio_put(PIN_LAMBDA_RX, 0);

    printf("before TX\n");
    sx126x_set_tx(&ctx, 3000);
    printf("after TX\n");
    
}

void lora_pack(bmp280_data_t* bmp_data, adxl345_data_t* adxl_data, ds3231_data_t* ds_data, bool enable_bmp, bool enable_adxl, bool enable_rtc){
    // init_data
    lora_data.accX = DATA_INVALID;
    lora_data.accY = DATA_INVALID;
    lora_data.accZ = DATA_INVALID;
    lora_data.temp = DATA_INVALID;
    lora_data.pres = DATA_INVALID;
    lora_data.phase = 255;
    lora_data.unix_time = 0;
    lora_data.servo_status = 255;
    lora_data.status = 255;



    //define local var
    uint32_t unix_time;
    int16_t temp_w;
    int16_t pres_w;
    int16_t x_w;
    int16_t y_w;
    int16_t z_w;

    //write to packet
    if (enable_bmp){
        temp_w = encode_temp(bmp_data);
        pres_w = encode_pressure(bmp_data);

        lora_data.temp = temp_w;
        lora_data.pres = pres_w;

    }
    if(enable_adxl){
        x_w = encode_accel(adxl_data,X);
        y_w = encode_accel(adxl_data,Y);
        z_w = encode_accel(adxl_data,Z);

        lora_data.accX = x_w;
        lora_data.accY = y_w;
        lora_data.accZ = z_w;
    }

    if(enable_rtc){
        unix_time = convert_to_unix_time(ds_data);

        lora_data.unix_time = unix_time;
    }

    //Write and Define Phase,Servo, Status

}

bool lora_run(){
    sx_wait_busy();
    lora_send();
    sleep_ms(5);
    uint16_t irq;
    sx126x_get_irq_status(&ctx, &irq);

    sx126x_clear_irq_status(&ctx, irq);

    if (irq & SX126X_IRQ_TX_DONE) {
        //printf("TX DONE\n");
        return false;
    }

    return true;
}