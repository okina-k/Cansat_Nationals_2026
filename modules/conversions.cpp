#include "./GeneralHeader.hpp"
//#include <arm_acle.h>


//Define Clamp Values
#define C_INT32_MAX = 2147483647
#define C_INT16_MAX = 32767
#define C_UINT16_MAX = 65535
#define C_UINT8_MAX = 255

#define C_UINT_MIN = 0
#define C_INT16_MIN = -32768

#define SCALE_TEMP 100.0f
#define SCALE_PRESSURE 10.0f
#define SCALE_ACCEL    1000.0f

//Conversions for LoRa

uint32_t convert_to_unix_time(ds3231_data_t* data) {
    struct tm t;
    
    t.tm_sec  = data -> sec;
    t.tm_min  = data -> min;
    t.tm_hour = data -> hour;
    t.tm_mday = data -> day;
    t.tm_mon  = data -> mon - 1;   // 0~11
    t.tm_year = data -> year - 1900; // From Yr 1900
    t.tm_isdst = 0;             //No Summer Time
    
    
    return (uint32_t)mktime(&t); // Convert to Unix Time
}



int16_t encode_temp(bmp280_data_t* data){
    float temp_c;

    temp_c = data -> temperature;

    return (int16_t)(temp_c * SCALE_TEMP + (temp_c >= 0 ? 0.5f : -0.5f));
}

uint16_t encode_pressure(bmp280_data_t* data) {
    float pressure_c;

    pressure_c = data -> pressure;

    pressure_c = pressure_c/100.0f;

    return (uint16_t)(pressure_c * SCALE_PRESSURE + (pressure_c >= 0 ? 0.5f : -0.5f));
}

uint16_t encode_pressure_callib(float pressure_pa) {
    // 1. Pa を hPa に変換 (101325.0 -> 1013.25)
    float pressure_hpa = pressure_pa / 100.0f;

    // 2. 小数点第1位まで保持するために10倍する (1013.25 -> 10132.5)
    // SCALE_PRESSURE が 10 の場合
    float scaled = pressure_hpa * SCALE_PRESSURE;

    // 3. 四捨五入して uint16_t にキャスト
    // 10132.5 -> 10133 (これなら 65535 以下に収まる)
    return (uint16_t)(scaled + (scaled >= 0 ? 0.5f : -0.5f));
}
int16_t encode_accel(adxl345_data_t* data, accel_choice choice ) {
    float acc_c;

    switch (choice){
        case X:
            acc_c = data -> x;
            break;
        case Y:
            acc_c = data -> y;
            break;
        case Z:
            acc_c = data -> z;
            break;
    }


    return (int16_t)(acc_c * SCALE_ACCEL + (acc_c >= 0 ? 0.5f : -0.5f));
}

float get_combined_acc(adxl345_data_t* data){
    float x,y,z;

    x = data -> x;
    y = data -> y;
    z = data -> z;

    return (float)sqrt(z*z+y*y+x*x);
}

//decode for reference

float decode_temperature(int16_t raw_val) {
    return (float)raw_val / SCALE_TEMP;
}

float decode_pressure(int16_t raw) {
    return (float)raw / SCALE_PRESSURE;
}

float decode_accel(int16_t raw) {
    return (float)raw / SCALE_ACCEL;
}

float get_isa_alt(float pres_hpa) {
    // callib_pres_t が Pa 単位なら 100.0 で割って hPa にする
    // すでに hPa ならそのまま使う
    double P0 = (double)callib_pres_t; 
    if (P0 > 5000.0) P0 /= 100.0; // Pa単位(10万)ならhPa(1000)に変換

    const double T0 = 288.15;    // [K]
    const double L  = 0.0065;    // [K/m]
    const double G  = 9.80665;   // [m/s^2]
    const double R_air = 287.052; // [J/kg・K] 空気の比気体定数

    // 指数部分: (R_air * L) / G ≒ 0.190263
    double exponent = (R_air * L) / G;
    
    // powの中身も hPa 単位で揃える
    double p = (double)pres_hpa;
    if (p > 5000.0) p /= 100.0; 

    double altitude = (T0 / L) * (1.0 - pow(p / P0, exponent));
    
    return (float)altitude;
}