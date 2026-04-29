#include "hw_config.h"
#include "sd_card.h"

// SPIインスタンス
static spi_t my_spi0 = {
    .hw_inst = spi0,
    .miso_gpio = 4,
    .mosi_gpio = 3,
    .sck_gpio  = 2,
    .baud_rate = 400 * 1000,
    .spi_mode=0,
    .no_miso_gpio_pull_up = false,
    .set_drive_strength = true,
    .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_8MA,
    .sck_gpio_drive_strength  = GPIO_DRIVE_STRENGTH_8MA,
    .use_static_dma_channels = false,
};

// SDカード設定
static sd_spi_if_t spi_if = {
    .spi = &my_spi0,
    .ss_gpio = 5
};

/* Configuration of the SD Card socket object */
static sd_card_t sd_card = {
    .type = SD_IF_SPI,
    .spi_if_p = &spi_if  // Pointer to the SPI interface driving this card
};

/* ********************************************************************** */

size_t sd_get_num() { return 1; }

/**
 * @brief Get a pointer to an SD card object by its number.
 *
 * @param[in] num The number of the SD card to get.
 *
 * @return A pointer to the SD card object, or @c NULL if the number is invalid.
 */
sd_card_t *sd_get_by_num(size_t num) {
    if (0 == num) {
        // The number 0 is a valid SD card number.
        // Return a pointer to the sd_card object.
        return &sd_card;
    } else {
        // The number is invalid. Return @c NULL.
        return NULL;
    }
}