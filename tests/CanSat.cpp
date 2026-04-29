#include "GeneralHeader.hpp"
#include <hardware/gpio.h>
#include <hardware/structs/io_bank0.h>
#include <pico/time.h>


#define DS3231_ADDR 0x68

adxl345_handle_t adxl;
bmp280_handle_t bmp;
FATFS fs;
FIL file;
UINT bw;
static float prev_alt = -1000;
static int descent_count = 0;
static int shock_count=0;

adxl345_data_t adxl_data;
bmp280_data_t bmp_data;
ds3231_data_t ds_data;
sx126x_pico_context_t ctx = {
    .spi = spi0,
    .cs_pin = PIN_LAMBDA_CS,
    .busy_pin = PIN_LAMBDA_BUSY,
    .reset_pin = PIN_LAMBDA_RESET
};

int log_count = 0;
float P0 = 101325;


//Bus Init

void set_spi_mode0(){
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_0,  
        SPI_CPHA_0,
        SPI_MSB_FIRST);
}


void set_spi_mode1(){
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_1,  
        SPI_CPHA_1,
        SPI_MSB_FIRST);
}



void spi_bus_init(){
    spi_init(SPI_PORT, 1000*1000);

    gpio_set_function(PIN_SCK,GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO,GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI,GPIO_FUNC_SPI);

    set_spi_mode0();
}

void i2c_bus_init(){
    i2c_init(I2C_PORT, 400*1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void init_sd() {

    pico_fatfs_spi_config_t config = {
        spi0,
        100 * KHZ,
        50 * MHZ,
        4,  // MISO
        18,  // CS
        2,  // SCK
        3,  // MOSI
        true
    };

    pico_fatfs_set_config(&config);
}

void servo_init() {
    gpio_set_function(PIN_SERVO_REV, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(PIN_SERVO_REV);

    pwm_set_clkdiv(slice, 125.0);  // 1us刻み
    pwm_set_wrap(slice, 20000);    // 20ms周期

    pwm_set_enabled(slice, true);
}


//LoRa Init

void lora_init(const sx126x_pico_context_t* ctx)
{
    sx126x_reset(ctx);
    sx126x_wakeup(ctx);

    /* standby */
    sx126x_set_standby(ctx, SX126X_STANDBY_CFG_RC);

    /* DC-DC regulator */
    sx126x_set_reg_mode(ctx, SX126X_REG_MODE_DCDC);

    /* LoRa packet */
    sx126x_set_pkt_type(ctx, SX126X_PKT_TYPE_LORA);

    /* FIFO base address */
    sx126x_set_buffer_base_address(ctx, 0x00, 0x00);

    /* RF frequency (868 MHz) */
    sx126x_set_rf_freq(ctx, 868000000);

    /* PA config */
    sx126x_pa_cfg_params_t pa_config = {
        .pa_duty_cycle = 0x04,
        .hp_max = 0x07,
        .device_sel = 0x00,
        .pa_lut = 0x01
    };

    sx126x_set_pa_cfg(ctx, &pa_config);

    /* TX power */
    sx126x_set_tx_params(ctx, 14, SX126X_RAMP_200_US);

    /* LoRa modulation parameters */
    sx126x_mod_params_lora_t mod_params = {
        .sf = SX126X_LORA_SF9,
        .bw = SX126X_LORA_BW_125,
        .cr = SX126X_LORA_CR_4_5,
        .ldro = 0
    };

    sx126x_set_lora_mod_params(ctx, &mod_params);

    /* LoRa packet parameters */
    sx126x_pkt_params_lora_t pkt_params = {
        .preamble_len_in_symb = 8,
        .header_type = SX126X_LORA_PKT_EXPLICIT,
        .pld_len_in_bytes = 64,
        .crc_is_on = true,
        .invert_iq_is_on = false
    };

    sx126x_set_lora_pkt_params(ctx, &pkt_params);

    /* IRQ設定 */
    sx126x_set_dio_irq_params(
        ctx,
        SX126X_IRQ_TX_DONE | SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT,
        SX126X_IRQ_TX_DONE | SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT,
        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );
}

//SD Card


void mount_sd() {

    FRESULT fr = f_mount(&fs, "", 1);

    if (fr == FR_OK) {
        printf("mount ok\n");
    } else {
        printf("mount error\n");
    }
}


void sd_open(){
    f_open(&file, "flight_record.csv", FA_WRITE | FA_OPEN_APPEND| FA_CREATE_ALWAYS);
}


void sd_write(const char* text){
    UINT bw;

    f_write(&file, text, strlen(text), &bw);

    log_count++;

    if(log_count >= 10){
        f_sync(&file);
        log_count = 0;
    }
}

void sd_close(){
    f_close(&file);
}

//Sensor Inits

void dummy_receive_callback(uint8_t type)
{
    
}

bool bmp280_usr_init(){
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
        printf("BMP280 init failed\n");
        return false;
    }

    printf("BMP280 init OK\n");

    
    bmp280_set_temperatue_oversampling(&bmp, BMP280_OVERSAMPLING_x2);
    bmp280_set_pressure_oversampling(&bmp, BMP280_OVERSAMPLING_x16);
    bmp280_set_filter(&bmp, BMP280_FILTER_COEFF_16);
    bmp280_set_standby_time(&bmp, BMP280_STANDBY_TIME_0P5_MS);
    bmp280_set_mode(&bmp, BMP280_MODE_NORMAL);

    return true;
}

bool adxl345_usr_init(){

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
    
    if (adxl345_init(&adxl) != 0)
    {
        printf("ADXL345 init failed\n");
        return false;
    }

    printf("ADXL345 init OK\n");


    adxl345_set_range(&adxl, ADXL345_RANGE_16G);
    adxl345_set_full_resolution(&adxl, ADXL345_BOOL_TRUE);
    adxl345_set_rate(&adxl, ADXL345_RATE_100);
    adxl345_set_measure(&adxl, ADXL345_BOOL_TRUE);


    return true;
}

//Read Values

uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

bool ds3231_read_time(ds3231_data_t *data) {
    uint8_t reg = 0x00;
    uint8_t buffer[7];

    // レジスタ指定
    if (i2c_write_blocking(I2C_PORT, DS3231_ADDR, &reg, 1, true) < 0)
        return false;

    // 7バイト読む
    if (i2c_read_blocking(I2C_PORT, DS3231_ADDR, buffer, 7, false) < 0)
        return false;

    data->sec  = bcd_to_dec(buffer[0] & 0x7F);
    data->min  = bcd_to_dec(buffer[1]);
    data->hour = bcd_to_dec(buffer[2] & 0x3F);
    data->day  = bcd_to_dec(buffer[4]);
    data->mon  = bcd_to_dec(buffer[5] & 0x1F);
    data->year = 2000 + bcd_to_dec(buffer[6]);

    return true;
}

bool bmp280_usr_read(bmp280_data_t *data){
    uint32_t temp_raw;
    uint32_t press_raw;
    float temperature;
    float pressure;


    if (bmp280_read_temperature_pressure(&bmp,
        &temp_raw,
        &temperature,
        &press_raw,
        &pressure) == 0){
        printf("Temp: %.2f C  Press: %.2f Pa\n",temperature, pressure);
        printf("temp_raw=%lu press_raw=%lu\n", temp_raw, press_raw);
    }
    else{
        return false;
    }

    data->temperature = temperature;
    data-> pressure = pressure;

    return true;
}

bool adxl345_usr_read(adxl345_data_t *data){
    set_spi_mode1();
    int16_t raw[1][3];
    float g[1][3];
    uint16_t len = 1;

    if (adxl345_read(&adxl, raw, g, &len) == 0){

        data-> x = g[0][0];
        data-> y = g[0][1];
        data-> z = g[0][2];
        printf("X: %.3f Y: %.3f Z: %.3f\n",
               g[0][0], g[0][1], g[0][2]);
               set_spi_mode0();
               sleep_ms(10);
    }
    else{
        printf("ADXL read error!\n");
        set_spi_mode0();
        return false;
    } 
    return true;
}

void buzzer_on(){
    gpio_put(PIN_BUZZER_SIG, 1);
}
void buzzer_off(){
    gpio_put(PIN_BUZZER_SIG,0);
}

void servo_write(int angle)
{
    int pulse = 1000 + (angle * 1000 / 180);
    pwm_set_gpio_level(PIN_SERVO_REV, pulse);
}


void lora_write(const sx126x_pico_context_t* ctx, const uint8_t* data, uint8_t len)
{
    sx126x_write_buffer(ctx, 0x00, data, len);

    sx126x_set_tx(ctx, 0);

    sx126x_irq_mask_t irq_mask;

    while (1)
    {
        sx126x_get_irq_status(ctx, &irq_mask);

        if (irq_mask & SX126X_IRQ_TX_DONE)
            break;

        sleep_ms(5);
    }

    sx126x_clear_irq_status(ctx, SX126X_IRQ_TX_DONE);
    //sx126x_set_rx(ctx, 0);
}

void lora_start_rx(const sx126x_pico_context_t* ctx)
{
    // 受信開始（0 = 無期限受信）
    sx126x_set_rx(ctx, 0);
}


uint8_t lora_read(const sx126x_pico_context_t* ctx, uint8_t* buffer)
{
    sx126x_rx_buffer_status_t status;

    // FIFO の状態取得
    sx126x_get_rx_buffer_status(ctx, &status);

    // データを読み出す
    sx126x_read_buffer(ctx,
                       status.buffer_start_pointer,
                       buffer,
                       status.pld_len_in_bytes);

    return status.pld_len_in_bytes;  // 読み出したバイト数を返す
}


float calc_altitude(float pressure, float P0)
{
    return 44330.0f * (1.0f - pow(pressure / P0, 0.1903f));
}

float get_acc(float x, float y, float z){
    return sqrt(x*x+y*y+z*z);
}


bool detect_apogee(float altitude, float acc)
{

    if(prev_alt == -1000){
        prev_alt = altitude;
        return false;
    }

    if(altitude < prev_alt){
        descent_count++;
    }
    else{
        descent_count = 0;
    }

    prev_alt = altitude;

    if(descent_count >= 5){
        return true;
    }

    if(acc > 8.0){
        shock_count++;
    }else{
        shock_count = 0;
    }
    
    if(shock_count >= 2){
        return true;
    }

    return false;
}


//Phases

bool pre_launch(){
    bool in_state_1=true;
    int i=0;
    int switch_count=0;
        while(in_state_1==true)
    {
        ds3231_read_time(&ds_data);

        adxl345_usr_read(&adxl_data);


        bmp280_usr_read(&bmp_data);

        float altitude = calc_altitude(bmp_data.pressure, P0);


        if(i%2==0){
            char log_line[128];
            snprintf(log_line,sizeof(log_line),
                "%04d-%02d-%02d %02d:%02d:%02d,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n",
                ds_data.year,
                ds_data.mon,
                ds_data.day,
                ds_data.hour,
                ds_data.min,
                ds_data.sec,
                adxl_data.x,
                adxl_data.y,
                adxl_data.z,
                altitude,
                bmp_data.temperature,
                bmp_data.pressure
            );
        
            sd_write(log_line);
        
            printf("%s", log_line);
        }

        sleep_ms(500);
        i++;

        float acc=get_acc(adxl_data.x,adxl_data.y,adxl_data.z);

        if(altitude > 5 && acc > 2.5)
        {
            switch_count++;
            if(switch_count>=3){
                in_state_1=false;
            }
        }
        else{
            switch_count=0;
        }
    }
    return true;
}

bool ascension(){
    bool in_state_2=true;


    int log_counter = 0;

    while(in_state_2==true){
        ds3231_read_time(&ds_data);

        adxl345_usr_read(&adxl_data);


        bmp280_usr_read(&bmp_data);

        float altitude = calc_altitude(bmp_data.pressure, P0);

        float acc = get_acc(
            adxl_data.x,
            adxl_data.y,
            adxl_data.z
        );

        if(log_counter >= 5){
            char log_line[128];
            snprintf(log_line,sizeof(log_line),
                "%04d-%02d-%02d %02d:%02d:%02d,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n",
                ds_data.year,
                ds_data.mon,
                ds_data.day,
                ds_data.hour,
                ds_data.min,
                ds_data.sec,
                adxl_data.x,
                adxl_data.y,
                adxl_data.z,
                altitude,
                bmp_data.temperature,
                bmp_data.pressure
            );
        
            sd_write(log_line);
        
            printf("%s", log_line);
            log_counter = 0;
        }
        if(detect_apogee(altitude, acc)){
            in_state_2 = false;
        }

        log_counter++;

        sleep_ms(100);
    }
    return true;

}

bool separation(){

    for(int i=0;i<20;i++){
        ds3231_read_time(&ds_data);

        adxl345_usr_read(&adxl_data);


        bmp280_usr_read(&bmp_data);

        float altitude = calc_altitude(bmp_data.pressure, P0);
        
        if(i%5==0){
            char log_line[128];
            snprintf(log_line,sizeof(log_line),
                "%04d-%02d-%02d %02d:%02d:%02d,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n",
                ds_data.year,
                ds_data.mon,
                ds_data.day,
                ds_data.hour,
                ds_data.min,
                ds_data.sec,
                adxl_data.x,
                adxl_data.y,
                adxl_data.z,
                altitude,
                bmp_data.temperature,
                bmp_data.pressure
            );
        
            sd_write(log_line);
        }

        sleep_ms(100);
    }
    return true;


}

bool descension(){
    bool in_state_4=true;
    int counter=0;
    float alt_before=0;
    int alt_count=0;
    lora_start_rx(&ctx);
    while(in_state_4==true){
        ds3231_read_time(&ds_data);

        adxl345_usr_read(&adxl_data);


        bmp280_usr_read(&bmp_data);

        float altitude = calc_altitude(bmp_data.pressure, P0);
        float acc = get_acc(
            adxl_data.x,
            adxl_data.y,
            adxl_data.z
        );
        
        if(counter%5==0){
            char log_line[128];
            snprintf(log_line,sizeof(log_line),
                "%04d-%02d-%02d %02d:%02d:%02d,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n",
                ds_data.year,
                ds_data.mon,
                ds_data.day,
                ds_data.hour,
                ds_data.min,
                ds_data.sec,
                adxl_data.x,
                adxl_data.y,
                adxl_data.z,
                altitude,
                bmp_data.temperature,
                bmp_data.pressure
            );
        
            sd_write(log_line);
        }
        if(counter%10==0){
            uint8_t packet[64];
            int len = snprintf((char*)packet,
            sizeof(packet),
            "%02d:%02d:%02d,%.1f,%.2f,%.1f,%.0f",
            ds_data.hour,
            ds_data.min,
            ds_data.sec,
            altitude,
            acc,
            bmp_data.temperature,
            bmp_data.pressure);

            lora_write(&ctx, packet, len);
        }
        if(alt_before==0){
            
        }
        else if(alt_before-altitude<1){
            alt_count++;
        }
        else{
            alt_count=0;
        }
        if(alt_count>=10){
            in_state_4=false;
        }
        alt_before=altitude;
        counter++;


        sleep_ms(100);
    }
    return true;
}

bool beacon(){
    bool in_state_5=true;
    buzzer_on();
    while(1){
        ds3231_read_time(&ds_data);

        adxl345_usr_read(&adxl_data);


        bmp280_usr_read(&bmp_data);

        float altitude = calc_altitude(bmp_data.pressure, P0);

        char log_line[128];
        snprintf(log_line,sizeof(log_line),
            "%04d-%02d-%02d %02d:%02d:%02d,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n",
            ds_data.year,
            ds_data.mon,
            ds_data.day,
            ds_data.hour,
            ds_data.min,
            ds_data.sec,
            adxl_data.x,
            adxl_data.y,
            adxl_data.z,
            altitude,
            bmp_data.temperature,
            bmp_data.pressure
        );

        sleep_ms(500);
    }
}

int main()
{

    stdio_init_all();
    sleep_ms(2000);
    gpio_init(PIN_BUZZER_SIG);
    gpio_set_function(PIN_BUZZER_SIG, GPIO_OUT);
    buzzer_on();
    //init
    spi_bus_init();
    i2c_bus_init();
   if( bmp280_usr_init()!=0){
        buzzer_off();
        while(1);
   }
   if(adxl345_usr_init()!=0){
        buzzer_off();
        while(1);
   }

    lora_init(&ctx);

    servo_init();

    mount_sd();
    sleep_ms(10);
    sd_open();

    for(int i=0; i<=4; i++){
        buzzer_on();
        sleep_ms(100);
        buzzer_off();
    }

    float pressure_buf[10];

    for(int i=0;i<10;i++){
        bmp280_usr_read(&bmp_data);
        pressure_buf[i] = bmp_data.pressure;
        sleep_ms(50);
    }

    float min_p = pressure_buf[0];
    float max_p = pressure_buf[0];
    float sum = 0;

    for(int i=0;i<10;i++){
        if(pressure_buf[i] < min_p) min_p = pressure_buf[i];
        if(pressure_buf[i] > max_p) max_p = pressure_buf[i];
        sum += pressure_buf[i];
    }

    if((max_p - min_p) < 5){
        P0 = sum / 10;
    }

    buzzer_off();

    pre_launch();
    ascension();
    separation();
    descension();
    beacon();






    printf("CanSat init OK\n");

}
