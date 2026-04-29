#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "sx126x.h"
#include "sx126x_hal.h"
#include "sx126x/defs.h"
#include "GeneralHeader.hpp"




const uint8_t tx_data[] = "hello,world";

sx126x_pico_context_t ctx = {
    .spi = spi0,
    .cs_pin = PIN_LAMBDA_CS,
    .busy_pin = PIN_LAMBDA_BUSY,
    .reset_pin = PIN_LAMBDA_RESET
};

sx126x_pkt_params_lora_t pkt_params = {
    .preamble_len_in_symb = 8,
    .header_type = SX126X_LORA_PKT_EXPLICIT,
    .pld_len_in_bytes = 11,
    .crc_is_on = true,
    .invert_iq_is_on = false
};

void spi_init_sx()
{
    spi_init(SPI_PORT, 1000 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_LAMBDA_CS);
    gpio_set_dir(PIN_LAMBDA_CS, GPIO_OUT);
    gpio_put(PIN_LAMBDA_CS, 1);

    gpio_init(PIN_LAMBDA_BUSY);
    gpio_set_dir(PIN_LAMBDA_BUSY, GPIO_IN);

    gpio_init(PIN_LAMBDA_RESET);
    gpio_set_dir(PIN_LAMBDA_RESET, GPIO_OUT);

    gpio_init(PIN_LAMBDA_DIO1);
    gpio_set_dir(PIN_LAMBDA_DIO1, GPIO_IN);
}
void lora_send()
{
    /* パケット長を再設定（重要） */
    sx126x_set_lora_pkt_params(&ctx, &pkt_params);

    /* バッファに書き込み */
    sx126x_write_buffer(&ctx, 0x00, tx_data, sizeof(tx_data) - 1);

    /* 送信開始 */
    sx126x_set_tx(&ctx, 0);
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    spi_init_sx();

    printf("Ground TX Start\n");

    sx126x_reset(&ctx);
    sx126x_wakeup(&ctx);

    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_RC);
    sx126x_set_reg_mode(&ctx, SX126X_REG_MODE_DCDC);
    sx126x_set_pkt_type(&ctx, SX126X_PKT_TYPE_LORA);
    sx126x_set_buffer_base_address(&ctx, 0x00, 0x00);

    /* 周波数（RXと一致させる） */
    sx126x_set_rf_freq(&ctx, 868300000);

    printf("pass1");

    /* PA設定 */
    sx126x_pa_cfg_params_t pa_config = {
        .pa_duty_cycle = 0x04,
        .hp_max = 0x07,
        .device_sel = 0x00,
        .pa_lut = 0x01
    };
    sx126x_set_pa_cfg(&ctx, &pa_config);

    sx126x_set_lora_sync_word(&ctx, 0x12);

    /* TXパワー */
    sx126x_set_tx_params(&ctx, 2, SX126X_RAMP_200_US);

    /* LoRa変調（RXと完全一致させる） */
    sx126x_mod_params_lora_t mod_params = {
        .sf = SX126X_LORA_SF9,
        .bw = SX126X_LORA_BW_125,
        .cr = SX126X_LORA_CR_4_5,
        .ldro = 0
    };
    sx126x_set_lora_mod_params(&ctx, &mod_params);

    /* パケット設定 */

    sx126x_set_lora_pkt_params(&ctx, &pkt_params);

    /* IRQ設定 */
    sx126x_set_dio_irq_params(
        &ctx,
        SX126X_IRQ_TX_DONE,
        SX126X_IRQ_TX_DONE,
        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );

    while (true)
    {
        printf("Sending...\n");

        sx126x_clear_irq_status(&ctx, SX126X_IRQ_ALL);
        lora_send();
        

        /* 送信完了待ち */
        uint16_t irq = 0;
        do {
            sx126x_get_irq_status(&ctx, &irq);
            if (irq & SX126X_IRQ_RX_DONE) printf("RX_DONE\n");
            if (irq & SX126X_IRQ_TX_DONE) printf("TX_DONE\n");
            if (irq & SX126X_IRQ_TIMEOUT) printf("TIMEOUT\n");
        } while ((irq & SX126X_IRQ_TX_DONE) == 0);
        printf("IRQ: %04X\n", irq);



        printf("Sent!\n");

        sleep_ms(1000); // 1秒ごと送信
    }
}