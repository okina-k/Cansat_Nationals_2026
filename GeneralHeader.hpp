#include <pico/time.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <cmath>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include <cstdint>
#include <time.h>

extern "C" {
    #include "adxl345/src/driver_adxl345.h"
    #include "adxl345/interface/driver_adxl345_interface.h"

    #include "bmp280/src/driver_bmp280.h"
    #include "bmp280/interface/driver_bmp280_interface.h"

    #include "ds3231/src/driver_ds3231.h"
    #include "ds3231/interface/driver_ds3231_interface.h"
    
    #include "src/sx126x.h"
    #include "fatfs/tf_card.h"
    #include "ff.h"
    #include "sx126x/defs.h"
}



// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_SCK  2
#define PIN_MOSI 3
#define PIN_MISO 4

//Define CS Pins
#define PIN_BMP_CS 6
#define PIN_ADXL_CS 7
#define PIN_LAMBDA_CS 10
#define PIN_SD_CS 18

//Define PWM/On_Off Pins
#define PIN_SERVO_ANT 21
#define PIN_SERVO_REV 20
#define PIN_BUZZER_SIG 22
#define PIN_LED_STDBY 26

//Define Aux
#define PIN_ADXL_INT1 8
#define PIN_ADXL_INT2 9

#define PIN_LAMBDA_RESET 11
#define PIN_LAMBDA_RX 12
#define PIN_LAMBDA_TX 13
#define PIN_LAMBDA_DIO1 14
#define PIN_LAMBDA_DIO2 15
#define PIN_LAMBDA_DIO3 16
#define PIN_LAMBDA_BUSY 17

#define PIN_SD_DET 19



// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1


struct bmp280_data_t{
    float temperature;
    float pressure;
};

struct adxl345_data_t{
    float x;
    float y;
    float z;
};

struct ds3231_data_t{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t mon;
    uint16_t year;
};

typedef struct __attribute__((packed)){
    uint32_t unix_time;
    int16_t temp;
    uint16_t pres;
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    uint8_t servo_status;
    uint8_t phase;
    uint8_t status;
} lora_payload_t;

enum io_choice{
    ADXL,
    BMP,
    RTC,
    LoRa
};

enum accel_choice{
    X,
    Y,
    Z
};
//Variables to include in all files




//Functions to include in all files
bool lora_run();
void spi_init_sx();
void i2c_init_sx();
void buzzer_init();
void buzzer_run();
void lora_init();
bool bmp_init();
bool adxl_init();
bool bmp_read(bmp280_data_t* data);
bool adxl_read(adxl345_data_t* data);
bool ds3231_read(ds3231_data_t* data);
void output_run(bmp280_data_t* bmp_data, adxl345_data_t* adxl_data, ds3231_data_t* ds_data, io_choice choice);
void lora_pack(bmp280_data_t* bmp_data, adxl345_data_t* adxl_data, ds3231_data_t* ds_data, bool enable_bmp, bool enable_adxl, bool enable_rtc);
uint32_t convert_to_unix_time(ds3231_data_t* data);
int16_t encode_temp(bmp280_data_t* data);
int16_t encode_pressure(bmp280_data_t* data);
int16_t encode_accel(adxl345_data_t* data, accel_choice choice);
float get_combined_acc(adxl345_data_t* data);
float decode_temperature(int16_t raw_val);
float decode_pressure(int16_t raw);
float decode_accel(int16_t raw);
