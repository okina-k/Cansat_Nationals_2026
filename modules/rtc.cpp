#include "../GeneralHeader.hpp"

#define DS3231_ADDR 0x68


uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

bool ds3231_set_time() {
    uint8_t data[8];

    data[0] = 0x00; 
    data[1] = dec_to_bcd(0);   // sec
    data[2] = dec_to_bcd(40);  // min
    data[3] = dec_to_bcd(20);  // time
    data[4] = dec_to_bcd(5);   // day of week
    data[5] = dec_to_bcd(5);  // date
    data[6] = dec_to_bcd(3);   // month
    data[7] = dec_to_bcd(26);  // year

    if (i2c_write_blocking(I2C_PORT, DS3231_ADDR, data, 8, false) < 0)
        return false;

    return true;
}

uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

bool ds3231_read(ds3231_data_t* data) {
    uint8_t reg = 0x00;
    uint8_t buffer[7];


    if (i2c_write_blocking(I2C_PORT, DS3231_ADDR, &reg, 1, true) < 0)
        return false;


    if (i2c_read_blocking(I2C_PORT, DS3231_ADDR, buffer, 7, false) < 0)
        return false;

    uint8_t sec  = bcd_to_dec(buffer[0] & 0x7F);
    uint8_t min  = bcd_to_dec(buffer[1]);
    uint8_t hour = bcd_to_dec(buffer[2] & 0x3F);
    uint8_t day  = bcd_to_dec(buffer[4]);
    uint8_t mon  = bcd_to_dec(buffer[5] & 0x1F);
    uint16_t year = 2000 + bcd_to_dec(buffer[6]);

    printf("%04u-%02u-%02u %02u:%02u:%02u\n",
           year, mon, day, hour, min, sec);
    
    data -> year = year;
    data -> mon = mon;
    data -> day = day;
    data -> hour = hour;
    data -> min = min;
    data -> sec = sec;
    
    return true;
}

