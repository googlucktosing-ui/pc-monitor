#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "ili9341_driver.h"
#include "lvgl/lvgl.h"

#define LCD_SPI_HOST    SPI2_HOST

static const char *TAG = "ili9341";
static esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
static lcd_flush_done_cb s_flush_done_cb = NULL;
static gpio_num_t s_bl_gpio = -1;

static bool notify_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    if (s_flush_done_cb)
        s_flush_done_cb(user_ctx);
    return false;
}

/* ILI9341 initialization sequence from BOE 2.8" IPS */
static void ili9341_init_seq(ili9341_cfg_t *cfg)
{
    vTaskDelay(pdMS_TO_TICKS(120));

    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xCF, (uint8_t[]){0x00, 0x89, 0x30}, 3);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xED, (uint8_t[]){0x67, 0x03, 0x12, 0x81}, 4);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xE8, (uint8_t[]){0x85, 0x01, 0x78}, 3);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xCB, (uint8_t[]){0x39, 0x2C, 0x00, 0x34, 0x02}, 5);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xF7, (uint8_t[]){0x20}, 1);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xEA, (uint8_t[]){0x00, 0x00}, 2);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xC0, (uint8_t[]){0x25}, 1);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xC1, (uint8_t[]){0x10}, 1);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xC5, (uint8_t[]){0x3F, 0x51}, 2);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xC7, (uint8_t[]){0xB0}, 1);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xB6, (uint8_t[]){0x0A, 0x82}, 2);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0x3A, (uint8_t[]){0x55}, 1);   // RGB565
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xF2, (uint8_t[]){0x00}, 1);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0x26, (uint8_t[]){0x01}, 1);

    // Set Gamma
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xE0,
        (uint8_t[]){0x0F, 0x27, 0x23, 0x0B, 0x0F, 0x05, 0x54, 0x74, 0x45, 0x0A, 0x17, 0x0A, 0x1C, 0x0E, 0x08}, 15);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0xE1,
        (uint8_t[]){0x08, 0x1A, 0x1E, 0x03, 0x0F, 0x05, 0x2E, 0x25, 0x3B, 0x01, 0x06, 0x05, 0x25, 0x33, 0x0F}, 15);

    // MADCTL: rotation
    uint8_t madctl = 0x48;  // default orientation
    switch (cfg->spin) {
        case 0: madctl = 0x48; break;  // 0 deg
        case 1: madctl = 0x28; break;  // 90 deg
        case 2: madctl = 0x88; break;  // 180 deg
        case 3: madctl = 0xE8; break;  // 270 deg
    }
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0x36, (uint8_t[]){madctl}, 1);

    esp_lcd_panel_io_tx_param(lcd_io_handle, 0x21, NULL, 0);   // INVON
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0x11, NULL, 0);   // SLPOUT
    vTaskDelay(pdMS_TO_TICKS(120));
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0x29, NULL, 0);   // DISPON
}

esp_err_t ili9341_driver_hw_init(ili9341_cfg_t *cfg)
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

    // Backlight GPIO
    gpio_config_t bl_cfg = {
        .pin_bit_mask = (1ULL << cfg->bl),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&bl_cfg);

    // Reset GPIO
    if (cfg->rst > 0) {
        gpio_config_t rst_cfg = {
            .pin_bit_mask = (1ULL << cfg->rst),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&rst_cfg);
    }

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
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, &lcd_io_handle));

    // Hardware reset
    if (cfg->rst > 0) {
        gpio_set_level(cfg->rst, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
        gpio_set_level(cfg->rst, 1);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    ili9341_init_seq(cfg);
    return ESP_OK;
}

void ili9341_flush(int x1, int x2, int y1, int y2, void *data)
{
    if (x2 <= x1 || y2 <= y1) {
        if (s_flush_done_cb) s_flush_done_cb(NULL);
        return;
    }
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0x2A,
        (uint8_t[]){(x1 >> 8) & 0xFF, x1 & 0xFF, ((x2 - 1) >> 8) & 0xFF, (x2 - 1) & 0xFF}, 4);
    esp_lcd_panel_io_tx_param(lcd_io_handle, 0x2B,
        (uint8_t[]){(y1 >> 8) & 0xFF, y1 & 0xFF, ((y2 - 1) >> 8) & 0xFF, (y2 - 1) & 0xFF}, 4);
    size_t len = (x2 - x1) * (y2 - y1) * 2;
    esp_lcd_panel_io_tx_color(lcd_io_handle, 0x2C, data, len);
}

void ili9341_lcd_backlight(bool enable)
{
    gpio_set_level(s_bl_gpio, enable ? 1 : 0);
}
