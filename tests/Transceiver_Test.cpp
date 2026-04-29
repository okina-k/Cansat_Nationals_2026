#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// 接続ピンに合わせて変更してください
#define SPI_PORT spi0
#define PIN_MISO 4
#define PIN_CS   17
#define PIN_SCK  2
#define PIN_MOSI 3
#define PIN_RESET 20 // リセットピン

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);

    // SPI初期化
    spi_init(SPI_PORT, 1000 * 1000); // 1MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // CSピンとRESETピンの初期化
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_init(PIN_RESET);
    gpio_set_dir(PIN_RESET, GPIO_OUT);
    gpio_put(PIN_RESET, 1); // リセット解除（High）

    printf("Starting SPI Test...\n");

    while (true) {
        // SX1262 Read Register (0x1D)
        // アドレス 0x0900 (Silicon Version) を読み出す
        uint8_t cmd[4] = {0x1D, 0x09, 0x00, 0x00}; 
        uint8_t val = 0xFF;

        gpio_put(PIN_CS, 0);
        spi_write_blocking(SPI_PORT, cmd, 4);
        spi_read_blocking(SPI_PORT, 0x00, &val, 1);
        gpio_put(PIN_CS, 1);

        printf("Read Result: 0x%02X\n", val);
        sleep_ms(1000);
    }
}