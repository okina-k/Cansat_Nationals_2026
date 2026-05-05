#include "GeneralHeader.hpp"



adxl345_data_t adxl_data;
bmp280_data_t bmp_data;
ds3231_data_t ds_data;
  
enable_flag flags;


int main(){

    flags.enable_bmp = true;
    flags.enable_adxl = true;
    flags.enable_rtc = false;
    flags.enable_lora = true;
    flags.enable_buzzer = true;
    flags.enable_sd = false;
    flags.enable_servo = true;
    flags.enable_io = true;

    //init
    stdio_init_all();
    if (flags.enable_io){
        while (!stdio_usb_connected()){
            sleep_ms(10);
        }
    }
    else{
        sleep_ms(2000);
        gpio_put(25,1);
        sleep_ms(1000);
        gpio_put(25,0);
    }
    

    //buzzer_on();
    
    spi_init_sx();
    i2c_init_sx();

    buzzer_init();

    sleep_ms(10);

    lora_init();
    if(flags.enable_bmp){
        if(bmp_init() != false){
            if(flags.enable_io){
                printf("BMP Init Failed");
            }
            //Handling
        }
    }
    if(flags.enable_adxl){
        if(adxl_init() != false){
            if(flags.enable_io){
                printf("ADXL Init Failed");
            }
            //Handling
        }
    }

    if(flags.enable_servo){
        servo_init();

    }

    if (flags.enable_buzzer){
        buzzer_run();
    }



    sleep_ms(2000);
    //buzzer_off();


    while(true){

        if(flags.enable_bmp==true){
            if(bmp_read(&bmp_data) != false){
                //Handling
            }
            else{
                
            }

            if (flags.enable_io){
                output_run(&bmp_data,&adxl_data,&ds_data,BMP);
            }
        }
        if(flags.enable_adxl==true){
            if(adxl_read(&adxl_data) != false){
                //Handling
            }
            else{

            }

            if (flags.enable_io){
                output_run(&bmp_data,&adxl_data,&ds_data,ADXL);
            }
        }
        if(flags.enable_rtc==true){
            if(ds3231_read(&ds_data) != false){
                //Handling
            }
            else{

            }

            if (flags.enable_io){
                output_run(&bmp_data,&adxl_data,&ds_data,RTC);
            }
            
        }
        
        if(flags.enable_lora==true){
            lora_pack(&bmp_data,&adxl_data,&ds_data,flags.enable_bmp,flags.enable_adxl,flags.enable_rtc);
            lora_run();
        }
        if(flags.enable_servo){
            servo_run(500, CTRCLOCK);
        }
        sleep_ms(2000);
    }

}

