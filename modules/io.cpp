#include "../GeneralHeader.hpp"
#include <cstdio>


void bmp_o(bmp280_data_t* data){
    float temperature;
    float pressure;

    temperature = data -> temperature;
    pressure = data -> pressure;

    printf("Temperature: %f\n",temperature);
    printf("Pressure: %f\n", pressure);
}

void adxl_o(adxl345_data_t* data){
    float x;
    float y;
    float z;

    x = data -> x;
    y = data -> y;
    z = data -> z;
    printf("AccX: %f\n",x);
    printf("AccY: %f\n",y);
    printf("AccZ: %f\n",z);
}

void rtc_o(ds3231_data_t* data){
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t mon;
    uint16_t year;

    sec = data -> sec;
    min = data -> min;
    hour = data -> hour;
    day = data -> day;
    mon = data -> mon;
    year = data -> year;

    printf("Year: %d\n",year);
    printf("Month: %d\n",mon);
    printf("Day: %d\n",day);
    printf("%d",hour);
    printf(" : ");
    printf("%d",min);
    printf(" : ");
    printf("%d\n",sec);
}


void output_run(bmp280_data_t* bmp_data, adxl345_data_t* adxl_data, ds3231_data_t* ds_data, io_choice choice){

    switch (choice){
        case ADXL:
            adxl_o(adxl_data);
            break;
    
        case BMP:
            bmp_o(bmp_data);
            break;

        case RTC:
            rtc_o(ds_data);
            break;

    }


}