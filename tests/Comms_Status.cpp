#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0
#define CS_PIN 5

void spi_init_test() {
    spi_init(SPI_PORT, 500 * 1000);

    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST);

    gpio_set_function(2, GPIO_FUNC_SPI);
    gpio_set_function(3, GPIO_FUNC_SPI);
    gpio_set_function(4, GPIO_FUNC_SPI);

    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);
    gpio_put(CS_PIN, 1);
    gpio_pull_up(CS_PIN);
}

uint8_t sx1262_get_status_raw() {
    uint8_t cmd = 0xC0;
    uint8_t status = 0;

    gpio_put(CS_PIN, 0);
    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_read_blocking(SPI_PORT, 0x00, &status, 1);
    gpio_put(CS_PIN, 1);

    return status;
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    spi_init_test();

    while (1) {
        uint8_t s = sx1262_get_status_raw();
        printf("SPI status = 0x%02X\n", s);
        sleep_ms(500);
    }
}