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
 * @file      driver_adxl345_interface_template.c
 * @brief     driver adxl345 interface template source file
 * @version   2.0.0
 * @author    Shifeng Li
 * @date      2021-04-20
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2021/04/20  <td>2.0      <td>Shifeng Li  <td>format the code
 * <tr><td>2020/12/23  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_adxl345_interface.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
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
uint8_t adxl345_interface_iic_init(void)
{
    return 1;
}

/**
 * @brief  interface iic bus deinit
 * @return status code
 *         - 0 success
 *         - 1 iic deinit failed
 * @note   none
 */
uint8_t adxl345_interface_iic_deinit(void)
{
    return 1;
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
uint8_t adxl345_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return 1;
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
uint8_t adxl345_interface_iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return 1;
}

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t adxl345_interface_spi_init(void)
{
    gpio_init(PIN_ADXL_CS);
    gpio_set_dir(PIN_ADXL_CS, GPIO_OUT);
    gpio_put(PIN_ADXL_CS, 1);
    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t adxl345_interface_spi_deinit(void)
{   
    return 0;
}

/**
 * @brief      interface spi bus read
 * @param[in]  reg register address
 * @param[out] *buf pointer to a data buffer
 * @param[in]  len length of data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t adxl345_interface_spi_read(uint8_t reg, uint8_t *buf, uint16_t len)
{

    spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    spi_set_baudrate(spi0, 1000000);

    uint8_t header = reg | 0x80;
    if (len > 1)
        header |= 0x40;

    gpio_put(PIN_ADXL_CS, 0);

    spi_write_blocking(SPI_PORT, &header, 1);
    spi_read_blocking(SPI_PORT, 0x00, buf, len);

    gpio_put(PIN_ADXL_CS, 1);


    return 0;
}

/**
 * @brief     interface spi bus write
 * @param[in] reg register address
 * @param[in] *buf pointer to a data buffer
 * @param[in] len length of data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t adxl345_interface_spi_write(uint8_t reg, uint8_t *buf, uint16_t len)
{
    
    spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
    spi_set_baudrate(spi0, 1000000);

    uint8_t header = reg & 0x3F;
    if (len > 1)
        header |= 0x40;

    gpio_put(PIN_ADXL_CS, 0);

    spi_write_blocking(SPI_PORT, &header, 1);
    spi_write_blocking(SPI_PORT, buf, len);

    gpio_put(PIN_ADXL_CS, 1);
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void adxl345_interface_delay_ms(uint32_t ms)
{
    sleep_ms(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void adxl345_interface_debug_print(const char *const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

/**
 * @brief     interface receive callback
 * @param[in] type irq type
 * @note      none
 */
void adxl345_interface_receive_callback(uint8_t type)
{
    switch (type)
    {
        case ADXL345_INTERRUPT_DATA_READY :
        {
            adxl345_interface_debug_print("adxl345: irq data ready.\n");
            
            break;
        }
        case ADXL345_INTERRUPT_SINGLE_TAP :
        {
            adxl345_interface_debug_print("adxl345: irq single tap.\n");
            
            break;
        }
        case ADXL345_INTERRUPT_DOUBLE_TAP :
        {
            adxl345_interface_debug_print("adxl345: irq double tap.\n");
            
            break;
        }
        case ADXL345_INTERRUPT_ACTIVITY :
        {
            adxl345_interface_debug_print("adxl345: irq activity.\n");
            
            break;
        }
        case ADXL345_INTERRUPT_INACTIVITY :
        {
            adxl345_interface_debug_print("adxl345: irq inactivity.\n");
            
            break;
        }
        case ADXL345_INTERRUPT_FREE_FALL :
        {
            adxl345_interface_debug_print("adxl345: irq free fall.\n");
            
            break;
        }
        case ADXL345_INTERRUPT_WATERMARK :
        {
            adxl345_interface_debug_print("adxl345: irq water mark.\n");
            
            break;
        }
        case ADXL345_INTERRUPT_OVERRUN :
        {
            adxl345_interface_debug_print("adxl345: irq overrun.\n");
            
            break;
        }
        default :
        {
            adxl345_interface_debug_print("adxl345: unknown code.\n");
            
            break;
        }
    }
}
