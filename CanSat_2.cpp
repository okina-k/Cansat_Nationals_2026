#include "GeneralHeader.hpp"
#include <cstdio>
#include <pico/time.h>


adxl345_data_t adxl_data;
bmp280_data_t bmp_data;
ds3231_data_t ds_data;

bool enable_bmp = true;
bool enable_adxl = true;
bool enable_rtc = false;
bool enable_lora=true;
bool enable_io=true;

void buzzer_on(){
    gpio_put(PIN_BUZZER_SIG,1);
}

void buzzer_off(){
    gpio_put(PIN_BUZZER_SIG, 0);
}

int main(){

    //init
    if (enable_io){
        stdio_init_all();
        while (!stdio_usb_connected()){
            sleep_ms(10);
        }
    }
    else{
        gpio_put(25,1);
        sleep_ms(100);
        gpio_put(25,0);
    }

    //buzzer_on();
    
    spi_init_sx();
    i2c_init_sx();

    buzzer_init();

    sleep_ms(10);

    lora_init();
    if(bmp_init() != false){
        if(enable_io){
            printf("BMP Init Failed");
        }
        //Handling
    }
    if(adxl_init() != false){
        if(enable_io){
            printf("ADXL Init Failed");
        }
        //Handling
    }

    buzzer_run();

    sleep_ms(2000);
    sleep_ms(10);
    //buzzer_off();


    while(true){

        if(enable_bmp==true){
            if(bmp_read(&bmp_data) != false){
                //Handling
            }
            else{
                
            }

            if (enable_io){
                output_run(&bmp_data,&adxl_data,&ds_data,BMP);
            }
        }
        if(enable_adxl==true){
            if(adxl_read(&adxl_data) != false){
                //Handling
            }
            else{

            }

            if (enable_io){
                output_run(&bmp_data,&adxl_data,&ds_data,ADXL);
            }
        }
        if(enable_rtc==true){
            if(ds3231_read(&ds_data) != false){
                //Handling
            }
            else{

            }

            if (enable_io){
                output_run(&bmp_data,&adxl_data,&ds_data,RTC);
            }
            
        }
        
        if(enable_lora==true){
            lora_pack(&bmp_data,&adxl_data,&ds_data,enable_bmp,enable_adxl,enable_rtc);
            lora_run();
        }
        sleep_ms(2000);
    }

}

