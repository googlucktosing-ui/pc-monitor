#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "st7789_driver.h"
#include "lvgl/lvgl.h"

#define LCD_SPI_HOST    SPI2_HOST

static const char *TAG = "st7789";
static esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
static esp_lcd_panel_handle_t lcd_panel_handle = NULL;
static lcd_flush_done_cb s_flush_done_cb = NULL;
static gpio_num_t s_bl_gpio = -1;

static bool notify_flush_ready(esp_lcd_panel_io_handle_t panel_io,
                               esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    if (s_flush_done_cb)
        s_flush_done_cb(user_ctx);
    return false;
}

esp_err_t st7789_driver_hw_init(st7789_cfg_t *cfg)
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = cfg->clk,
        .mosi_io_num = cfg->mosi,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .flags = SPICOMMON_BUSFLAG_MASTER,
        .max_transfer_sz = cfg->width * 40 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    s_flush_done_cb = cfg->done_cb;
    s_bl_gpio = cfg->bl;

    gpio_config_t bl_cfg = {
        .pin_bit_mask = (1ULL << cfg->bl),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&bl_cfg);

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = cfg->dc,
        .cs_gpio_num = cfg->cs,
        .pclk_hz = cfg->spi_fre,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = notify_flush_ready,
        .user_ctx = cfg->cb_param,
        .flags = { .sio_mode = 0 },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST,
                                              &io_config, &lcd_io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = cfg->rst,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io_handle, &panel_config, &lcd_panel_handle));

    esp_lcd_panel_reset(lcd_panel_handle);
    esp_lcd_panel_init(lcd_panel_handle);
    /* data_endian=LITTLE in panel_config tells the ST7789 driver to set RAMCTRL automatically */
    esp_lcd_panel_invert_color(lcd_panel_handle, true);
    esp_lcd_panel_set_gap(lcd_panel_handle, 0, 0);
    esp_lcd_panel_disp_on_off(lcd_panel_handle, true);

    return ESP_OK;
}

void st7789_flush(int x1, int x2, int y1, int y2, void *data)
{
    if (x2 <= x1 || y2 <= y1) {
        if (s_flush_done_cb) s_flush_done_cb(NULL);
        return;
    }
    esp_lcd_panel_draw_bitmap(lcd_panel_handle, x1, y1, x2, y2, data);
}

void st7789_lcd_backlight(bool enable)
{
    gpio_set_level(s_bl_gpio, enable ? 1 : 0);
}
