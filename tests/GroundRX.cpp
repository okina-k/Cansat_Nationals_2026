#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "sx126x.h"
#include "sx126x_hal.h"
 #include "sx126x/defs.h"

#include "GeneralHeader.hpp"
#define BUFFER_SIZE 256 

volatile bool dio1_flag = false;

uint8_t rx_buffer[BUFFER_SIZE];

sx126x_pico_context_t ctx = {
    .spi = spi0,
    .cs_pin = PIN_LAMBDA_CS,
    .busy_pin = PIN_LAMBDA_BUSY,
    .reset_pin =  PIN_LAMBDA_RESET
};

void spi_init_sx()
{
    spi_init(SPI_PORT, 1000 * 1000); // 1MHz

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
    gpio_pull_down(PIN_LAMBDA_DIO1);
}



void lora_start_rx()
{
    sx126x_set_rx(&ctx, 0xFFFFFF); // continuous RX
}

void dio1_callback(uint gpio, uint32_t events)
{
    dio1_flag = true;
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);


    while (!stdio_usb_connected())
    {
        sleep_ms(10);
    }

    spi_init_sx();

    gpio_set_irq_enabled_with_callback(
        PIN_LAMBDA_DIO1,
        GPIO_IRQ_EDGE_RISE,
        true,
        &dio1_callback
    );

    printf("Ground RX Start\n");

    sx126x_reset(&ctx);
    sx126x_wakeup(&ctx);

    printf("pass1");

    /* standby */
    sx126x_set_standby(&ctx, SX126X_STANDBY_CFG_RC);

    /* DC-DC regulator */
    sx126x_set_reg_mode(&ctx, SX126X_REG_MODE_DCDC);

    /* LoRa packet */
    sx126x_set_pkt_type(&ctx, SX126X_PKT_TYPE_LORA);

    printf("pass2");

    /* FIFO base address */
    sx126x_set_buffer_base_address(&ctx, 0x00, 0x00);

    /* RF frequency (868 MHz) */
    sx126x_set_rf_freq(&ctx, 868300000);

    printf("pass3");




    printf("pass4");

    /* LoRa modulation parameters */
    sx126x_mod_params_lora_t mod_params = {
        .sf = SX126X_LORA_SF9,
        .bw = SX126X_LORA_BW_125,
        .cr = SX126X_LORA_CR_4_5,
        .ldro = 0
    };

    sx126x_set_lora_mod_params(&ctx, &mod_params);

    sx126x_set_lora_sync_word(&ctx, 0x12);

    /* LoRa packet parameters */
    sx126x_pkt_params_lora_t pkt_params = {
        .preamble_len_in_symb = 8,
        .header_type = SX126X_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes = 255,
        .crc_is_on = true,
        .invert_iq_is_on = false
    };

    sx126x_set_lora_pkt_params(&ctx, &pkt_params);


    
    //IRQ設定 
    sx126x_set_dio_irq_params(
        &ctx,
        SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT | SX126X_IRQ_CRC_ERROR,
        SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT | SX126X_IRQ_CRC_ERROR,
        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );
    /*
    sx126x_set_dio_irq_params(
        &ctx,
        0xFFFF,   // ←全部IRQ有効化
        0xFFFF,
        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );
    */
    lora_start_rx();

    printf("pass5");

    while (true)
    {
        printf("Waiting for IRQ...\n");

        dio1_flag = true;

        if (dio1_flag)
        {
            dio1_flag = false;
    
            uint16_t irq = 0;
            sx126x_get_irq_status(&ctx, &irq);

            printf("IRQ: %04X\n", irq);
    
            if (irq == 0) continue;
    
            sx126x_clear_irq_status(&ctx, irq);
    
            if (irq & SX126X_IRQ_CRC_ERROR)
            {
                printf("CRC ERROR\n");
            }
    
            if (irq & SX126X_IRQ_RX_DONE)
            {
                printf("RX_DONE\n");
    
                if (!(irq & SX126X_IRQ_CRC_ERROR))
                {
                    sx126x_rx_buffer_status_t status;
                    sx126x_get_rx_buffer_status(&ctx, &status);
    
                    uint8_t len = status.pld_len_in_bytes;
                    uint8_t offset = status.buffer_start_pointer;
    
                    if(len >= BUFFER_SIZE) len = BUFFER_SIZE - 1;
    
                    sx126x_read_buffer(&ctx, offset, rx_buffer, len);
                    rx_buffer[len] = 0;
    
                    printf("RX: %s\n", rx_buffer);
                }
    
                lora_start_rx();
            }
    
            if (irq & SX126X_IRQ_TIMEOUT) printf("TIMEOUT\n");
        }
    
        sleep_ms(10);
    }
}