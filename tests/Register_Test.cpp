#include <hardware/irq.h>
#include <pico/time.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "GeneralHeader.hpp"

sx126x_pico_context_t ctx = {
    .spi = spi0,
    .cs_pin = PIN_LAMBDA_CS,
    .busy_pin = PIN_LAMBDA_BUSY,
    .reset_pin =  PIN_LAMBDA_RESET
};

volatile bool dio1_flag = false;

void set_spi_mode0(){
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_0,  
        SPI_CPHA_0,
        SPI_MSB_FIRST);
}

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

uint8_t read_reg_sx1262(uint16_t address) {
    uint8_t command = 0x1D;             // Read Register Command
    uint8_t addr_h = (address >> 8) & 0xFF;
    uint8_t addr_l = address & 0xFF;
    uint8_t dummy = 0x00;               // ダミーバイト（読み出し用クロック）
    uint8_t result = 0;

    int timeout = 0;
    while (gpio_get(PIN_LAMBDA_BUSY)) {
        sleep_ms(1);
        timeout++;
        if (timeout > 100) {
            printf("ERROR: BUSY Timeout\n");
            return 0xFF; // エラーコード
        }
    }
    // -----
    // 少しだけ待つ（念のため）
    sleep_us(50); 

    // 2. CSをLOWにしてトランザクション開始
    gpio_put(PIN_LAMBDA_CS, 0);

    // 3. コマンドとアドレスを送信
    spi_write_blocking(SPI_PORT, &command, 1);
    spi_write_blocking(SPI_PORT, &addr_h, 1);
    spi_write_blocking(SPI_PORT, &addr_l, 1);
    
    // 4. ダミーバイトを送信しながら、その戻り値としてデータを読み込む
    // Read Registerの場合、アドレス送信の後にダミーを送り、その直後にデータが返る
    spi_write_blocking(SPI_PORT, &dummy, 1); // 読み出し開始用のダミー
    spi_read_blocking(SPI_PORT, 0x00, &result, 1); // 実際のデータ読み取り

    // 5. CSをHIGHにしてトランザクション終了
    gpio_put(PIN_LAMBDA_CS, 1);

    return result;
}

void lora_init() {
    sx126x_reset(&ctx);
    sleep_ms(10);

    sx126x_wakeup(&ctx);
    sleep_ms(10);

    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_XOSC);
    sleep_ms(2);
    sx126x_cal(&ctx, SX126X_CAL_ALL);

    sx126x_set_pkt_type(&ctx, SX126X_PKT_TYPE_LORA);


    sx126x_set_rf_freq(&ctx, 868000000);


    sx126x_mod_params_lora_t mod = {
        .sf = SX126X_LORA_SF7,
        .bw = SX126X_LORA_BW_125,
        .cr = SX126X_LORA_CR_4_5,
        .ldro = 0
    };
    sx126x_set_lora_mod_params(&ctx, &mod);

    sx126x_pkt_params_lora_t pkt = {
        .preamble_len_in_symb = 8,
        .header_type = SX126X_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes = 10, // ★ここ重要
        .crc_is_on = true,
        .invert_iq_is_on = false
    };
    sx126x_set_lora_pkt_params(&ctx, &pkt);



    sx126x_set_buffer_base_address(&ctx, 0x00, 0x80);

    sx126x_set_dio_irq_params(
        &ctx,
        SX126X_IRQ_ALL,
        SX126X_IRQ_RX_DONE,
        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );
    sx126x_set_lora_sync_word(&ctx, 0x34);
    sx126x_set_dio2_as_rf_sw_ctrl(&ctx, false);
    //sx126x_set_dio3_as_tcxo_ctrl(&ctx, SX126X_TCXO_CTRL_NONE, 0);
}

void loop() {
    sx126x_chip_status_t status;
    
    // 現在の状態を取得
    sx126x_get_status(&ctx, &status);
    
    printf("Chip Mode: %d | Cmd Status: %d\n", 
            status.chip_mode, 
            status.cmd_status);
            
    // 状態の目安:
    // 2 = Standby RC
    // 3 = Standby XOSC
    // 4 = FS
    // 5 = RX
    // 6 = TX
    
    sleep_ms(1000);
}


int main(){
    stdio_init_all();

    while (!stdio_usb_connected())
    {
        sleep_ms(10);
    }

    spi_init_sx();
    sleep_ms(20);

    gpio_set_irq_enabled_with_callback(
        PIN_LAMBDA_DIO1,
        GPIO_IRQ_EDGE_RISE,
        true,
        &dio1_irq_handler
    );

    printf("Pass2\n");

    lora_init();

        // mainの中
    gpio_put(PIN_LAMBDA_RESET, 0);
    sleep_ms(10); // 十分に待つ
    gpio_put(PIN_LAMBDA_RESET, 1);
    sleep_ms(20); // 起動待ち

    // チップをWAKEUPさせるためのダミーSPI送信
    uint8_t cmd = 0xC0; // GetStatus
    gpio_put(PIN_LAMBDA_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    gpio_put(PIN_LAMBDA_CS, 1);

    sleep_ms(10);
// ここで read_reg(0x0900) を実行

    while(true){
        loop();
    }
}