#include "GeneralHeader.hpp"

bmp280_handle_t bmp;
adxl345_handle_t adxl;

void dummy_receive_callback(uint8_t type)
{
    
}

bool bmp_init(){

    memset(&bmp, 0, sizeof(bmp280_handle_t));

    bmp.iic_init = bmp280_interface_iic_init;
    bmp.iic_deinit = bmp280_interface_iic_deinit;
    bmp.iic_read = bmp280_interface_iic_read;
    bmp.iic_write = bmp280_interface_iic_write;

    bmp.spi_init = bmp280_interface_spi_init;
    bmp.spi_deinit = bmp280_interface_spi_deinit;
    bmp.spi_read = bmp280_interface_spi_read;
    bmp.spi_write = bmp280_interface_spi_write;

    bmp.delay_ms = bmp280_interface_delay_ms;
    bmp.debug_print = bmp280_interface_debug_print;

    bmp280_set_interface(&bmp, BMP280_INTERFACE_SPI);

    if (bmp280_init(&bmp) != 0)
    {
        return true;
    }


    
    bmp280_set_temperatue_oversampling(&bmp, BMP280_OVERSAMPLING_x2);
    bmp280_set_pressure_oversampling(&bmp, BMP280_OVERSAMPLING_x16);
    bmp280_set_filter(&bmp, BMP280_FILTER_COEFF_16);
    bmp280_set_standby_time(&bmp, BMP280_STANDBY_TIME_0P5_MS);
    bmp280_set_mode(&bmp, BMP280_MODE_NORMAL);
    
    return false;
}

bool adxl_init(){
    memset(&adxl, 0, sizeof(adxl345_handle_t));// 初期化

    adxl.iic_init = adxl345_interface_iic_init;
    adxl.iic_deinit = adxl345_interface_iic_deinit;
    adxl.iic_read = adxl345_interface_iic_read;
    adxl.iic_write = adxl345_interface_iic_write;
    adxl.receive_callback = dummy_receive_callback;

    adxl.spi_init = adxl345_interface_spi_init;
    adxl.spi_deinit = adxl345_interface_spi_deinit;
    adxl.spi_read = adxl345_interface_spi_read;
    adxl.spi_write = adxl345_interface_spi_write;
    adxl.delay_ms = adxl345_interface_delay_ms;
    adxl.debug_print = adxl345_interface_debug_print;
    adxl345_set_interface(&adxl, ADXL345_INTERFACE_SPI);

    int errorcode=adxl345_init(&adxl);

    if (errorcode!=0)
    {
        return true;
    }


    adxl345_set_range(&adxl, ADXL345_RANGE_16G);
    adxl345_set_full_resolution(&adxl, ADXL345_BOOL_TRUE);
    adxl345_set_rate(&adxl, ADXL345_RATE_100);
    adxl345_set_measure(&adxl, ADXL345_BOOL_TRUE);

    return false;

}

bool bmp_read(bmp280_data_t* data){
    uint32_t temp_raw;
    uint32_t press_raw;
    float temperature;
    float pressure;
    
    if (bmp280_read_temperature_pressure(&bmp,
                                         &temp_raw,
                                         &temperature,
                                         &press_raw,
                                         &pressure) == 0)
    {
        //
    }
    else
    {
        return true;
    }

    //write in data struct
    data -> pressure = pressure;
    data -> temperature = temperature;

    return false;
}

bool adxl_read(adxl345_data_t* data){
    int16_t raw[1][3];
    float g[1][3];
    uint16_t len = 1;

    if (adxl345_read(&adxl, raw, g, &len) == 0)
    {
        //
    }
    else{
        return true;
    } 

    //write in data struct
    data -> x =  g[0][0];
    data -> y =  g[0][1];
    data -> z =  g[0][2];

    return false;
}