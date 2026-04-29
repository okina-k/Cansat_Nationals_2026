#include "no-OS-FatFS-SD-SDIO-SPI-RPi-Pico-main/include/FatFsSD.h"
#include "diskio.h"
#include "sd_card.h"
#include <pico/time.h>
#include <stdio.h>
using namespace FatFsNs;

#define SPI_PORT spi0
#define PIN_MISO 4
#define PIN_CS   5
#define PIN_SCK  2
#define PIN_MOSI 3
// ダミークロック送信用ヘルパー
void send_dummy_clocks(int count) {
    uint8_t ff = 0xFF;
    for(int i = 0; i < count; i++) {
        spi_write_blocking(SPI_PORT, &ff, 1);
    }
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) { sleep_ms(100); }

    // 1. SPIの初期化 (最初は低速の100kHzで)
    spi_init(SPI_PORT, 100 * 1000); 
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    spi_set_format(SPI_PORT,
        8,
        SPI_CPOL_0,  
        SPI_CPHA_0,
        SPI_MSB_FIRST);

    // 2. CSピンの初期化
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 0); // 通信しないときはHIGH

    printf("Starting SD Card Init...\n");

    // 3. SDカードへ電源投入後のダミークロック (74クロック以上)
    // CSをHIGHにしたまま送信
    send_dummy_clocks(10); 

    // 4. CMD0 (GO_IDLE_STATE) 送信
    // SDカードをSPIモードに移行させる
    uint8_t cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
    uint8_t response = 0xFF;

    gpio_put(PIN_CS, 1); // CS ON
    spi_write_blocking(SPI_PORT, cmd0, 6);

    // 5. レスポンス待機 (R1形式: 0x01が返れば成功)
    for(int i = 0; i < 8; i++) {
        spi_read_blocking(SPI_PORT, 0xFF, &response, 1);
        printf("[%d] Response: 0x%02X\n", i, response);
        
        if (response == 0x01) {
            printf("Success: Card in Idle State.\n");
            break;
        }
    }
    
    
    gpio_put(PIN_CS, 1); // CS OFF
    printf("Init Finished.\n");

    while(1) {
        sleep_ms(1000);
    }

            /*
    FatFs::begin();

    SdCard* sd = FatFs::SdCard_get_by_num(0);
    sd->mount();

    File file;
    file.open("test.txt", FA_WRITE | FA_CREATE_ALWAYS);

    UINT bw;
    file.write("hello", 5, &bw);

    file.close();

    while (1);
    */

    /*

    if (!sd_init_driver()) {
        printf("SD init failed\n");
    } else {
        printf("SD init OK\n");
    }
    FATFS fs;
    FIL file;
    FRESULT fr;

    
    sd_card_t *p = sd_get_by_num(0);

    if (p->init(p) != 0) {
        printf("card init failed\n");
    } else {
        printf("card init success\n");
    }
    // マウント
    fr = f_mount(&fs, "0:", 1);

    printf("mount result: %d\n", fr);
    if (fr != FR_OK) {
        printf("mount error\n");
        return 1;
    }

    // ファイル作成
    fr = f_open(&file, "0:/test.txt", FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("open error\n");
        return 1;
    }

    // 書き込み
    UINT bw;
    f_write(&file, "Hello SD card!\n", 15, &bw);

    // 閉じる
    f_close(&file);

    printf("done\n");

    while (1);
    */
}