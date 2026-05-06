#include "GeneralHeader.hpp"
#include <cstdio>
#include <pico/time.h>



adxl345_data_t adxl_data;
bmp280_data_t bmp_data;
ds3231_data_t ds_data;
bool counter_thresh_1_done=false;
bool counter_thresh_2_done=false;
bool counter_thresh_3_done=false;
int counter_thresh_1=0;
int counter_thresh_2=0;
int counter_thresh_3=0;
float max_height=0;
  
enable_flag flags;

float callib_pres_t = 1013.25f;


int main(){

    flags.enable_bmp = true;
    flags.enable_adxl = true;
    flags.enable_rtc = false;

    flags.enable_lora = true;
    flags.enable_buzzer = true;
    flags.enable_sd = false;
    flags.enable_servo = true;
    flags.enable_io =false;

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

    if (flags.enable_bmp){
        double sum_callib=0.0;
        for (int i=0; i<10; i++){
            bmp_read(&bmp_data);
            sum_callib+=bmp_data.pressure;
            //printf("%d\n",sum_callib);
            sleep_ms(100);
        }
        callib_pres_t=(float)(sum_callib/10.0f);
    }





    sleep_ms(2000);
    //buzzer_off();

    float last_alt=0;
    int count_alt_delta=0;
    while(count_alt_delta<10){
        if(get_isa_alt(bmp_data.pressure)<last_alt and get_isa_alt(bmp_data.pressure)>200){
            count_alt_delta++;
        }
        else{
            count_alt_delta=0;
        }
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
        if(max_height<get_isa_alt(bmp_data.pressure)and get_isa_alt(bmp_data.pressure)<400){
            max_height=get_isa_alt(bmp_data.pressure);
        }
        sleep_ms(200);
    }
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

        
        if(get_isa_alt(bmp_data.pressure)<200 and max_height>200){
            counter_thresh_1++;
        }
        if(flags.enable_servo and counter_thresh_1>4 and counter_thresh_1_done==false){
            servo_run(1600, CTRCLOCK);
            counter_thresh_1_done=true;
        }
        if(get_isa_alt(bmp_data.pressure)<200 and max_height>200){
            counter_thresh_2++;
        }
        if(flags.enable_servo and counter_thresh_2>4 and counter_thresh_2_done==false and counter_thresh_1_done==true and max_height>200){
            servo_run(1600, CTRCLOCK);
            counter_thresh_2_done=true;
        }
        if(get_isa_alt(bmp_data.pressure)<200 and max_height>200){
            counter_thresh_3++;
        }
        if(flags.enable_servo and counter_thresh_3>4 and counter_thresh_3_done==false and counter_thresh_1_done==true and counter_thresh_2_done==true and max_height>200){
            servo_run(1600, CTRCLOCK);
            counter_thresh_3_done=true;
        }
        sleep_ms(100);
    }
    }


