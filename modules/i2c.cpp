#include "../GeneralHeader.hpp"

void i2c_init_sx() {
    i2c_init(I2C_PORT, 100*1000); // 100kHz

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}