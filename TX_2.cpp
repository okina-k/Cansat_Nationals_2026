#include <hardware/irq.h>
#include <pico/time.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "GeneralHeader.hpp"

extern "C" {
#include "sx126x.h"
#include "sx126x_hal.h"
}





volatile bool dio1_flag = false;

uint8_t tx_buf[] = "HELLO 868";
uint8_t tx_len = strlen((char*)tx_buf);  // ← 10

void set_spi_mode0(){
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_0,  
        SPI_CPHA_0,
        SPI_MSB_FIRST);
}


// IRQ
void dio1_irq_handler(uint gpio, uint32_t events) {
    dio1_flag = true;
}

void spi_init_sx()
{
    spi_init(SPI_PORT, 1000 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_LAMBDA_CS);
    gpio_set_dir(PIN_LAMBDA_CS, GPIO_OUT);
    gpio_put(PIN_LAMBDA_CS, 1);
    gpio_pull_up(PIN_LAMBDA_CS);

    gpio_init(PIN_LAMBDA_BUSY);
    gpio_set_dir(PIN_LAMBDA_BUSY, GPIO_IN);

    gpio_init(PIN_LAMBDA_RESET);
    gpio_set_dir(PIN_LAMBDA_RESET, GPIO_OUT);
    gpio_put(PIN_LAMBDA_RESET, 1);

    gpio_init(PIN_LAMBDA_DIO1);
    gpio_set_dir(PIN_LAMBDA_DIO1, GPIO_IN);



    gpio_init(PIN_LAMBDA_RX);
    gpio_set_dir(PIN_LAMBDA_RX, GPIO_OUT);


    gpio_init(PIN_LAMBDA_TX);
    gpio_set_dir(PIN_LAMBDA_TX, GPIO_OUT);


    gpio_put(PIN_LAMBDA_RX, 0);
    gpio_put(PIN_LAMBDA_TX, 0);


    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    set_spi_mode0();


}

uint8_t sx1262_get_status_raw() {
    uint8_t cmd = 0xC0;
    uint8_t status = 0;

    gpio_put(PIN_LAMBDA_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_read_blocking(SPI_PORT, 0x00, &status, 1);
    gpio_put(PIN_LAMBDA_CS, 1);

    return status;
}




void lora_init() {

    sx126x_reset(&ctx);

    sleep_ms(10);
    printf("Pass1\n");
    sx126x_wakeup(&ctx);

    sleep_ms(10);


    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_RC);
    sx126x_set_reg_mode(&ctx, SX126X_REG_MODE_LDO);   // ←追加
    sx126x_cal(&ctx, SX126X_CAL_ALL);  


    sx126x_set_pkt_type(&ctx, SX126X_PKT_TYPE_LORA);


        // ★ PA設定（これ無いと送信しないことある）
    sx126x_pa_cfg_params_t pa = {
        .pa_duty_cycle = 0x04,
        .hp_max = 0x07,
        .device_sel = 0x00, // SX1262
        .pa_lut = 0x01
    };

    // ★ 868MHz
    sx126x_set_rf_freq(&ctx, 868000000);

    sx126x_set_pa_cfg(&ctx, &pa);

    // ★ 出力パワー設定
    sx126x_set_tx_params(&ctx, 14, SX126X_RAMP_200_US);


    // LoRa設定
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
        .pld_len_in_bytes = 10,
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
        // Sync Word（重要）
    sx126x_set_lora_sync_word(&ctx, 0x34);
}

void lora_send() {

    while (gpio_get(PIN_LAMBDA_BUSY));
    sleep_ms(1);

    sx126x_write_buffer(&ctx, 0x00, tx_buf, tx_len);

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
int main() {
    stdio_init_all();

    while (!stdio_usb_connected())
    {
        sleep_ms(10);
    }

    spi_init_sx();

    printf("Pass1\n");

    // DIO1 IRQ（GPIO14）
    gpio_set_irq_enabled_with_callback(
        PIN_LAMBDA_DIO1,
        GPIO_IRQ_EDGE_RISE,
        true,
        &dio1_irq_handler
    );

    printf("Pass2\n");

    lora_init();
    sleep_us(10);

    gpio_put(PIN_LAMBDA_RX, 0);
    gpio_put(PIN_LAMBDA_TX, 1);
    sleep_ms(5);
    
    printf("Pass3\n");

    printf("Init_Done\n");

    while (true) {
        printf("loop\n");

        uint8_t s = sx1262_get_status_raw();
        printf("SPI status = 0x%02X\n", s);
        sleep_ms(500);

        while (gpio_get(PIN_LAMBDA_BUSY)) {
            tight_loop_contents();
        }
        sx126x_chip_status_t status;

        sx126x_get_status(&ctx, &status);
        
        printf("mode=%d cmd=%d\n",
               status.chip_mode,
               status.cmd_status);
        printf("Sending...\n");

        printf("pass1");

        printf("RX=%d TX=%d\n",
        gpio_get(PIN_LAMBDA_RX),
        gpio_get(PIN_LAMBDA_TX));


        lora_send();
        sleep_ms(5);
        sx126x_chip_status_t st;
        sx126x_get_status(&ctx, &st);

        printf("chip_mode=%d cmd_status=%d\n",
            st.chip_mode,
            st.cmd_status);


        printf("pass2");
        //while (!dio1_flag);
        //dio1_flag = false;

        printf("pass3");
        uint16_t irq;
        sx126x_get_irq_status(&ctx, &irq);

        printf("IRQ raw: 0x%04X\n", irq);
        sx126x_clear_irq_status(&ctx, irq);

        printf("pass4");


        if (irq & SX126X_IRQ_TX_DONE) {
            printf("TX DONE\n");
            gpio_put(25, 1);
            sleep_ms(100);
            gpio_put(25, 0);

        }
        printf("pass5\n");
        sleep_ms(10000);
        printf("pass6\n");
    }
}