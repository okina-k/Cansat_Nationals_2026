#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/spi.h"
extern "C" {
    #include "src/sx126x.h"
    }
#include "defs.h"


static sx126x_pico_context_t sx1262_ctx;

void sx1262_hw_init( void )
{
    sx1262_ctx.spi = spi0;
    sx1262_ctx.cs_pin = 10;
    sx1262_ctx.reset_pin = 11;
    sx1262_ctx.busy_pin = 12;
}

void sx1262_gpio_init(void)
{
    // CS
    gpio_init(sx1262_ctx.cs_pin);
    gpio_set_dir(sx1262_ctx.cs_pin, GPIO_OUT);
    gpio_put(sx1262_ctx.cs_pin, 1);

    // RESET
    gpio_init(sx1262_ctx.reset_pin);
    gpio_set_dir(sx1262_ctx.reset_pin, GPIO_OUT);
    gpio_put(sx1262_ctx.reset_pin, 1);

    // BUSY
    gpio_init(sx1262_ctx.busy_pin);
    gpio_set_dir(sx1262_ctx.busy_pin, GPIO_IN);
}

void sx1262_spi_init(void)
{
    spi_init(sx1262_ctx.spi, 1000 * 1000);
    spi_set_format(
        sx1262_ctx.spi,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST
    );

    gpio_set_function(3, GPIO_FUNC_SPI);
    gpio_set_function(4, GPIO_FUNC_SPI);
    gpio_set_function(5, GPIO_FUNC_SPI);
}

int main(void){

    
    stdio_init_all();

    sleep_ms(2000);  

    printf("RX_Mode\n");

    sx1262_hw_init();
    sx1262_gpio_init();
    sx1262_spi_init();

    sx126x_reset(&sx1262_ctx);
    sleep_ms(10);
    sx126x_set_standby(&sx1262_ctx, SX126X_STANDBY_CFG_RC);

    sx126x_set_pkt_type(&sx1262_ctx, SX126X_PKT_TYPE_LORA);
    sx126x_set_rf_freq(&sx1262_ctx, 868000000);

    sx126x_mod_params_lora_t mod = {
        .sf   = SX126X_LORA_SF7,
        .bw   = SX126X_LORA_BW_125,
        .cr   = SX126X_LORA_CR_4_5,
        .ldro = 0,
    };
    sx126x_set_lora_mod_params(&sx1262_ctx, &mod);

    sx126x_pkt_params_lora_t pkt = {
        .preamble_len_in_symb = 8,
        .header_type          = SX126X_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes     = 255,   // ★ RXは最大
        .crc_is_on            = 1,
        .invert_iq_is_on      = 0,
    };
    sx126x_set_lora_pkt_params(&sx1262_ctx, &pkt);

    sx126x_set_buffer_base_address(&sx1262_ctx, 0x00, 0x00);

    sx126x_set_dio_irq_params(
        &sx1262_ctx,
        SX126X_IRQ_RX_DONE | SX126X_IRQ_CRC_ERROR,
        SX126X_IRQ_RX_DONE | SX126X_IRQ_CRC_ERROR,
        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );

    sx126x_set_rx(&sx1262_ctx, 0);


    while (1)
    {
    sx126x_irq_mask_t irq;
    sx126x_get_irq_status(&sx1262_ctx, &irq);

    if (irq == SX126X_IRQ_NONE) {
        sleep_ms(5);
        continue;
    }
    
    sx126x_clear_irq_status(&sx1262_ctx, irq);

    if (irq & SX126X_IRQ_RX_DONE)
    {

        sx126x_rx_buffer_status_t rx_stat;
        sx126x_get_rx_buffer_status(&sx1262_ctx, &rx_stat);

        uint8_t buf[256] = {0};
        sx126x_read_buffer(
            &sx1262_ctx,
            rx_stat.buffer_start_pointer,
            buf,
            rx_stat.pld_len_in_bytes
        );

        printf("LEN=%d PTR=0x%02X\n",
            rx_stat.pld_len_in_bytes,
            rx_stat.buffer_start_pointer);

        printf("DATA: ");
        for (int i = 0; i < rx_stat.pld_len_in_bytes; i++) {
            printf("%02X ", buf[i]);
        }
        printf("\n");

        sx126x_set_rx(&sx1262_ctx, 0); 
    }

    if (irq & SX126X_IRQ_CRC_ERROR)
    {
        printf("CRC ERR\n");

        sx126x_clear_irq_status(&sx1262_ctx, SX126X_IRQ_CRC_ERROR);
    }

    sleep_ms(5);
    }
}