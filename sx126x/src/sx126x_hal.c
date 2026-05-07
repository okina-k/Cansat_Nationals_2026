#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "sx126x_hal.h"
#include "../defs.h"
#include <stdio.h>




static inline void sx126x_wait_while_busy(const sx126x_pico_context_t* ctx)
{
    while (gpio_get(ctx->busy_pin)) {
        tight_loop_contents();
    }
}

sx126x_hal_status_t sx126x_hal_reset( const void* context ){
    const sx126x_pico_context_t* ctx = context;

    gpio_put(ctx->reset_pin, 0);
    sleep_ms(1);          // datasheet: >100us
    gpio_put(ctx->reset_pin, 1);
    sleep_ms(10);         // 起動待ち

    sx126x_wait_while_busy(ctx);

    return SX126X_HAL_STATUS_OK;

}
sx126x_hal_status_t sx126x_hal_read(
    const void* context,
    const uint8_t* command,
    const uint16_t command_length,
    uint8_t* data,
    const uint16_t data_length )
{
    const sx126x_pico_context_t* ctx = (const sx126x_pico_context_t*)context;

    sx126x_wait_while_busy(ctx);

    gpio_put(ctx->cs_pin, 0);
    sleep_us(2);

    // CMD
    spi_write_blocking(ctx->spi, command, command_length);

    // ★重要：クロック生成はreadでやる
    if (data_length > 0) {
        spi_read_blocking(ctx->spi, 0x00, data, data_length);
    }

    gpio_put(ctx->cs_pin, 1);
    sleep_us(2);

    sx126x_wait_while_busy(ctx);

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_write( const void* context, const uint8_t* command, const uint16_t command_length,const uint8_t* data, const uint16_t data_length )
{
    const sx126x_pico_context_t* ctx = (const sx126x_pico_context_t*)context;

    sx126x_wait_while_busy(ctx);

    gpio_put(ctx->cs_pin, 0);


    spi_write_blocking(ctx->spi, command, command_length);


    if (data_length > 0 && data != NULL) {
        spi_write_blocking(ctx->spi, data, data_length);
    }

    gpio_put(ctx->cs_pin, 1);

    sx126x_wait_while_busy(ctx);

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup( const void* context )
{
    const sx126x_pico_context_t* ctx =
        (const sx126x_pico_context_t*)context;

    uint8_t cmd = 0xC0;

    //printf("Pass0\n");

    gpio_put(ctx->cs_pin, 0);
    spi_write_blocking(ctx->spi, &cmd, 1);
    gpio_put(ctx->cs_pin, 1);
    //printf("Pass1\n");

    sleep_us(150);

    sx126x_wait_while_busy(ctx);

    //printf("Pass2\n");

    return SX126X_HAL_STATUS_OK;
}