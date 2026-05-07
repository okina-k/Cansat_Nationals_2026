/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_ds3231_interface_template.c
 * @brief     driver ds3231 interface template source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-03-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/03/15  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/11/30  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_ds3231_interface.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "../../GeneralHeader.hpp"
#include <pico/time.h>
#include <stdio.h>
#include <stdarg.h>

/**
 * @brief  interface iic bus init
 * @return status code
 *         - 0 success
 *         - 1 iic init failed
 * @note   none
 */
uint8_t ds3231_interface_iic_init(void)
{
    i2c_init(I2C_PORT, DS_I2C_BAUD);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    return 0;
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t ds3231_interface_iic_deinit(void)
{
    i2c_deinit(I2C_PORT);
    return 0;
}

/**
 * @brief      interface iic bus read
 * @param[in]  addr iic device write address
 * @param[in]  reg iic register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t ds3231_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
        uint8_t dev_addr = addr >> 1;  // 8bit → 7bit変換

    if (i2c_write_blocking(I2C_PORT, dev_addr, &reg, 1, true) < 0)
        return 1;

    if (i2c_read_blocking(I2C_PORT, dev_addr, buf, len, false) < 0)
        return 1;

    return 0;
    return 0;
}

/**
 * @brief     interface iic bus write
 * @param[in] addr iic device write address
 * @param[in] reg iic register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t ds3231_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    uint8_t dev_addr = addr >> 1;

    uint8_t temp[len + 1];
    temp[0] = reg;
    for (uint16_t i = 0; i < len; i++)
    {
        temp[i + 1] = buf[i];
    }

    if (i2c_write_blocking(I2C_PORT, dev_addr, temp, len + 1, false) < 0)
        return 1;

    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void ds3231_interface_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void ds3231_interface_debug_print(const char *const fmt, ...)
{
    
}

/**
 * @brief     interface receive callback
 * @param[in] type interrupt type
 * @note      none
 */
void ds3231_interface_receive_callback(uint8_t type)
{
    switch (type)
    {
        case DS3231_STATUS_ALARM_2 :
        {
            ds3231_interface_debug_print("ds3231: irq alarm2.\n");
            
            break;
        }
        case DS3231_STATUS_ALARM_1 :
        {
            ds3231_interface_debug_print("ds3231: irq alarm1.\n");
            
            break;
        }
        default :
        {
            break;
        }
    }
}
