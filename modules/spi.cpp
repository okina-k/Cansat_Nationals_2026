#include "../GeneralHeader.hpp"
#include <hardware/gpio.h>


void set_spi_mode0(){
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_0,  
        SPI_CPHA_0,
        SPI_MSB_FIRST);
}


void set_spi_mode3(){
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_1,  
        SPI_CPHA_1,
        SPI_MSB_FIRST);
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

    gpio_init(PIN_BMP_CS);
    gpio_set_dir(PIN_BMP_CS, GPIO_OUT);
    gpio_put(PIN_BMP_CS, 1);

    gpio_init(PIN_ADXL_CS);
    gpio_set_dir(PIN_ADXL_CS, GPIO_OUT);
    gpio_put(PIN_ADXL_CS, 1);


    gpio_put(PIN_LAMBDA_RX, 0);
    gpio_put(PIN_LAMBDA_TX, 0);


    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    gpio_init(PIN_BUZZER_SIG);
    gpio_set_dir(PIN_BUZZER_SIG,GPIO_OUT);
    gpio_put(PIN_BUZZER_SIG,1);

    set_spi_mode0();
}