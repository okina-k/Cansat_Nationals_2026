#include <hardware/gpio.h>
#include <pico/time.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "GeneralHeader.hpp"

extern "C" {
    #include "sx126x.h"
    #include "sx126x_hal.h"
}

lora_payload_t rx_data;
uint8_t rx_buf[sizeof(lora_payload_t)];

sx126x_pico_context_t ctx = {
    .spi = spi0,
    .cs_pin = PIN_LAMBDA_CS,
    .busy_pin = PIN_LAMBDA_BUSY,
    .reset_pin =  PIN_LAMBDA_RESET
};

void set_spi_mode0(){
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_0,  
        SPI_CPHA_0,
        SPI_MSB_FIRST);
}

bool dio1_flag = false;
volatile bool spi_lock = false;



void dio1_irq_handler(uint gpio, uint32_t events) {
    dio1_flag = true;
}

// 簡易テスト：SX1262のSilicon Versionレジスタ(0x0900)を読み出す
uint8_t read_test() {
    uint8_t cmd = 0x1D; // Read Register
    uint8_t addr_h = 0x09;
    uint8_t addr_l = 0x00;
    uint8_t dummy = 0x00;
    uint8_t val = 0;

    gpio_put(PIN_LAMBDA_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_write_blocking(SPI_PORT, &addr_h, 1);
    spi_write_blocking(SPI_PORT, &addr_l, 1);
    spi_write_blocking(SPI_PORT, &dummy, 1); // ダミー読み出し
    spi_read_blocking(SPI_PORT, 0x00, &val, 1); // 値の読み出し
    gpio_put(PIN_LAMBDA_CS, 1);
    printf("%d\n",val);
    return val;
}

void spi_init_sx()
{
    spi_init(SPI_PORT, 100 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_LAMBDA_CS);
    gpio_set_dir(PIN_LAMBDA_CS, GPIO_OUT);
    gpio_put(PIN_LAMBDA_CS, 1);
    //gpio_disable_pulls(PIN_LAMBDA_CS);

    gpio_init(PIN_LAMBDA_BUSY);
    gpio_set_dir(PIN_LAMBDA_BUSY, GPIO_IN);
    gpio_set_input_hysteresis_enabled(PIN_LAMBDA_BUSY, true);

    gpio_init(PIN_LAMBDA_RESET);
    gpio_set_dir(PIN_LAMBDA_RESET, GPIO_OUT);
    gpio_put(PIN_LAMBDA_RESET, 1);
    //gpio_pull_up(PIN_LAMBDA_RESET);

    gpio_init(PIN_LAMBDA_DIO1);
    gpio_set_dir(PIN_LAMBDA_DIO1, GPIO_IN);
    //gpio_pull_down(PIN_LAMBDA_DIO1);
    //gpio_disable_pulls(PIN_LAMBDA_DIO1);


    gpio_init(PIN_LAMBDA_RX);
    gpio_set_dir(PIN_LAMBDA_RX, GPIO_OUT);

    gpio_init(PIN_LAMBDA_TX);
    gpio_set_dir(PIN_LAMBDA_TX, GPIO_OUT);

    gpio_put(PIN_LAMBDA_RX, 0);
    gpio_put(PIN_LAMBDA_TX, 0);

    set_spi_mode0();
}

static inline void sx_wait_busy()
{
    while (gpio_get(PIN_LAMBDA_BUSY)) {
        tight_loop_contents();
    }
    sleep_us(200);
}


static void sx1262_safe_get_status(sx126x_chip_status_t *st)
{


    sx_wait_busy();
    sx126x_get_status(&ctx, st);

}



void print_status(){
    // 修正前: modeとcmdを表示していた部分を以下に変更
    uint8_t status_byte = 0;
        // 念のため、GetStatusコマンドを一度だけ生で投げる
        // (ライブラリを使わずに直接SPIで取得してみる)
    uint8_t cmd_get_status = 0xC0;
    gpio_put(PIN_LAMBDA_CS, 0);
    spi_write_blocking(SPI_PORT, &cmd_get_status, 1);
    spi_read_blocking(SPI_PORT, 0x00, &status_byte, 1);
    gpio_put(PIN_LAMBDA_CS, 1);
    
    printf("RAW STATUS BYTE: 0x%02X\n", status_byte);
}

void lora_init() {
    sx126x_chip_status_t st;
    sx126x_reset(&ctx);
    sleep_ms(10);



    sx126x_wakeup(&ctx);
    sleep_ms(10);
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    printf("Reset Status: 0x%02X:\n");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();
    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_XOSC);
    sleep_ms(10);
    //sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_RC);
    printf("stdby");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();





    sleep_ms(2);
    sx126x_cal(&ctx, SX126X_CAL_ALL);


    printf("cal ");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();

    sx126x_set_pkt_type(&ctx, SX126X_PKT_TYPE_LORA);

    printf("plt ");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();



    sx126x_set_rf_freq(&ctx, 868000000);



    printf("set frq ");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();


    sx126x_mod_params_lora_t mod = {
        .sf = SX126X_LORA_SF7,
        .bw = SX126X_LORA_BW_125,
        .cr = SX126X_LORA_CR_4_5,
        .ldro = 0
    };
    sx_wait_busy();
    sx126x_set_lora_mod_params(&ctx, &mod);

    printf("set mod param ");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();

    sx126x_pkt_params_lora_t pkt = {
        .preamble_len_in_symb = 16,
        .header_type = SX126X_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes = 17, 
        .crc_is_on = true,
        .invert_iq_is_on = false
    };
    sx_wait_busy();
    sx126x_set_lora_pkt_params(&ctx, &pkt);

    printf("set pkt prm ");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();
    sx126x_set_buffer_base_address(&ctx, 0x00, 0x80);

    printf("set base");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();
    sx126x_set_dio_irq_params(
        &ctx,
        SX126X_IRQ_ALL,
        SX126X_IRQ_RX_DONE,
        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );

    printf("set irq");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();
    sx126x_set_lora_sync_word(&ctx, 0x34);

    printf("set sync");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    sx_wait_busy();
    sx126x_set_dio2_as_rf_sw_ctrl(&ctx, false);

    printf("set dio2 rf");
    sx_wait_busy();
    sx126x_get_status(&ctx, &st);
    print_status();
    //sx126x_set_dio3_as_tcxo_ctrl(&ctx, SX126X_TCXO_CTRL_NONE, 0);
}


void data_io(){
    uint32_t unix_time;
    int16_t temp_raw;
    uint16_t pres_raw;
    int16_t accX_raw;
    int16_t accY_raw;
    int16_t accZ_raw;
    uint8_t servo_status;
    uint8_t phase;
    uint8_t status;

    unix_time = rx_data.unix_time;
    temp_raw = rx_data.temp;
    pres_raw = rx_data.pres;
    accX_raw = rx_data.accX;
    accY_raw = rx_data.accY;
    accZ_raw = rx_data.accZ;
    servo_status = rx_data.servo_status;
    phase = rx_data.phase;
    status = rx_data.status;

    printf("Temp_Raw: %d\n",temp_raw);
    printf("Pres_Raw: %d\n",pres_raw);
    printf("AccX_Raw: %d\n",accX_raw);
    printf("AccY_Raw: %d\n",accY_raw);
    printf("AccZ_Raw: %d\n",accZ_raw);

    float temp;
    float pres;
    float accX;
    float accY;
    float accZ;

    temp = decode_temperature(temp_raw);
    pres = decode_pressure(pres_raw);
    accX = decode_accel(accX_raw);
    accY = decode_accel(accY_raw);
    accZ = decode_accel(accZ_raw);

    printf("Temp: %f\n",temp);
    printf("Pres: %f\n",pres);
    printf("AccX: %f\n",accX);
    printf("AccY: %f\n",accY);
    printf("AccZ: %f\n",accZ);
}



// 受信データ処理専用の関数
void process_rx_data(uint16_t irq_status) {
    if (!(irq_status & SX126X_IRQ_RX_DONE)) return;

    // 1. バッファステータスの取得
    sx126x_rx_buffer_status_t rx_status;
    sx126x_get_rx_buffer_status(&ctx, &rx_status);

    
    // データがない場合は何もしない
    if (rx_status.pld_len_in_bytes == 0) return;

    // 2. データの読み出し

    sx126x_read_buffer(&ctx, rx_status.buffer_start_pointer, rx_buf, rx_status.pld_len_in_bytes);

    memcpy(&rx_data, rx_buf, sizeof(lora_payload_t));

    data_io();
    //Hex read raw
    printf("Hex Data: ");
    for(int i = 0; i < rx_status.pld_len_in_bytes; i++) {
        printf("%02X ", rx_buf[i]);
    }
    printf("\n");


}

/*
void start_rx() {

    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_RC); // XOSCではなくRCで一度リセット
    sx_wait_busy();
    sleep_ms(10); // 十分待つ


    sx126x_clear_irq_status(&ctx, 0xFFFF);

    printf("clear IRQ");
    print_status();

    printf("[RX] waiting BUSY...\n");
    sx_wait_busy();
    sleep_us(100);

    gpio_put(PIN_LAMBDA_RX, 1);
    gpio_put(PIN_LAMBDA_TX, 0);

    sx_wait_busy();

    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_XOSC);

    printf("set stdby");
    print_status();

    // ★重要：ここ追加
    sx_wait_busy();
    sleep_us(100);

    printf("[RX] set_rx...\n");
    sx126x_set_rx(&ctx, 0xFFFFFF);

    printf("set RX");
    print_status();
    
    // RXモードを確認した直後にこれを入れる
    uint16_t irq_status = 0;
    sx126x_get_irq_status(&ctx, &irq_status);
    printf("IRQ STATUS after fail: 0x%04X\n", irq_status);


    sx_wait_busy();

    sx126x_chip_status_t st;
    sx126x_get_status(&ctx, &st);

    printf("RX MODE CHECK: mode=%d cmd=%d\n",
        st.chip_mode,
        st.cmd_status);


    printf("[RX] started\n");
}
    */




void start_rx() {
    // 1. スタンバイRCで確実に状態をクリア
    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_RC); 
    sx_wait_busy();

    // 2. パケットタイプ再設定
    sx126x_set_pkt_type(&ctx, SX126X_PKT_TYPE_LORA);
    sx_wait_busy();

    // 3. IRQ クリア
    sx126x_clear_irq_status(&ctx, 0xFFFF);
    sx_wait_busy();

    // 4. RFスイッチをRXへ（受信開始前）
    gpio_put(PIN_LAMBDA_TX, 0);
    gpio_put(PIN_LAMBDA_RX, 1);

    // 5. RXセット（タイムアウトを短く設定してみる）
    sx126x_set_rx(&ctx, 1000); // 1秒タイムアウト
    sx_wait_busy();

    // --- ここで確認 ---
    sx126x_chip_status_t st;
    sx126x_get_status(&ctx, &st);
    printf("RX MODE CHECK: mode=%d (Expected 5), cmd=%d\n", st.chip_mode, st.cmd_status);
}

// メインループや受信待機関数内
void wait_for_packet() {
    uint16_t irq_status = 0;
    
    // 現在の状態を確認するために取得
    sx126x_get_irq_status(&ctx, &irq_status);
    /*
    if (irq_status != 0) {
        printf("IRQ Detected! Value: 0x%04X\n", irq_status);
    }
        */

    if (irq_status & SX126X_IRQ_RX_DONE) {
        printf("RX DONE！\n");
        process_rx_data(irq_status);
        sx126x_clear_irq_status(&ctx, irq_status );
        start_rx();
    } 
    else if (irq_status & SX126X_IRQ_TIMEOUT) {
        printf("Timeout\n");
        // タイムアウトしたら再開するため、必要ならここで再セット
        sx126x_clear_irq_status(&ctx, irq_status );
        start_rx();
    }

}
int main()
{
    stdio_init_all();

    while (!stdio_usb_connected())
    {
        sleep_ms(10);
    }
    printf("pass\n");

    spi_init_sx();
    sleep_ms(100);
    lora_init();
    sleep_ms(100);
    start_rx();

    gpio_set_irq_enabled(PIN_LAMBDA_DIO1, GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_callback(&dio1_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);

    read_test();
    while (true) {
       
        /*
        if (dio1_flag) {
            printf("pass");
            dio1_flag = false;
        
            uint16_t irq;
            sx126x_get_irq_status(&ctx, &irq);
            printf("IRQ: 0x%04X\n", irq);
        
            if (irq & SX126X_IRQ_RX_DONE) {
                printf("RX DONE\n");
            
                sx126x_rx_buffer_status_t status;
                sx126x_get_rx_buffer_status(&ctx, &status);
            
                uint8_t len = status.pld_len_in_bytes;
                uint8_t offset = status.buffer_start_pointer;
            
                sx126x_read_buffer(&ctx, offset, rx_buf, len);
            
                printf("LEN: %d\n", len);
            }
        
            if (irq & SX126X_IRQ_CRC_ERROR) {
                printf("CRC ERROR\n");
            }
        
            sx126x_clear_irq_status(&ctx, irq);
        }
        sx126x_set_rx(&ctx, 0xFFFFFF);
        */
        wait_for_packet();
        sleep_us(100);
    }
}