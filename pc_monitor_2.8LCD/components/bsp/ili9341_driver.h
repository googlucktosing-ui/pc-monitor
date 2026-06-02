#ifndef _ILI9341_DRIVER_H_
#define _ILI9341_DRIVER_H_
#include "driver/gpio.h"
#include "esp_err.h"

typedef void(*lcd_flush_done_cb)(void* param);

typedef struct
{
    gpio_num_t  mosi;       // SPI MOSI
    gpio_num_t  clk;        // SPI CLK
    gpio_num_t  cs;         // SPI CS
    gpio_num_t  dc;         // Data/Command
    gpio_num_t  rst;        // Reset
    gpio_num_t  bl;         // Backlight
    uint32_t    spi_fre;    // SPI clock (Hz)
    uint16_t    width;      // Display width
    uint16_t    height;     // Display height
    uint8_t     spin;       // Rotation (0=0°, 1=90°, 2=180°, 3=270°)
    lcd_flush_done_cb   done_cb;    // Flush done callback
    void*       cb_param;   // Callback param
} ili9341_cfg_t;

esp_err_t ili9341_driver_hw_init(ili9341_cfg_t* cfg);
void ili9341_flush(int x1, int x2, int y1, int y2, void *data);
void ili9341_lcd_backlight(bool enable);

#endif
