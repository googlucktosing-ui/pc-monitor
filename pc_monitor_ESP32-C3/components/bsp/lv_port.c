#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "lv_port.h"
#include "lvgl.h"
#include "ili9341_driver.h"

static lv_disp_drv_t disp_drv;
static const char *TAG = "lv_port";

#define LCD_WIDTH   320
#define LCD_HEIGHT  240

static void lv_tick_inc_cb(void *data)
{
    uint32_t tick_inc_period_ms = *((uint32_t *)data);
    lv_tick_inc(tick_inc_period_ms);
}

static void lv_port_flush_ready(void *param)
{
    lv_disp_flush_ready(&disp_drv);
}

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    (void)disp_drv;
    ili9341_flush(area->x1, area->x2 + 1, area->y1, area->y2 + 1, color_p);
}

static void lv_port_disp_init(void)
{
    static lv_disp_draw_buf_t draw_buf_dsc;
    size_t disp_buf_height = 10;

    lv_color_t *p_disp_buf1 = heap_caps_malloc(LCD_WIDTH * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    lv_color_t *p_disp_buf2 = heap_caps_malloc(LCD_WIDTH * disp_buf_height * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    ESP_LOGI(TAG, "Allocate %u * %u display buffer, size:%u Byte", LCD_WIDTH, disp_buf_height, LCD_WIDTH * disp_buf_height * sizeof(lv_color_t) * 2);
    if (NULL == p_disp_buf1 || NULL == p_disp_buf2) {
        ESP_LOGE(TAG, "No memory for LVGL display buffer");
        esp_system_abort("Memory allocation failed");
    }

    lv_disp_draw_buf_init(&draw_buf_dsc, p_disp_buf1, p_disp_buf2, LCD_WIDTH * disp_buf_height);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf_dsc;
    lv_disp_drv_register(&disp_drv);
}

static void lv_port_tick_init(void)
{
    static uint32_t tick_inc_period_ms = 5;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = lv_tick_inc_cb,
        .name = "",
        .arg = &tick_inc_period_ms,
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = true,
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, tick_inc_period_ms * 1000));
}

static void lcd_init(void)
{
    ili9341_cfg_t cfg;
    cfg.mosi = GPIO_NUM_6;
    cfg.clk  = GPIO_NUM_7;
    cfg.cs   = GPIO_NUM_5;
    cfg.dc   = GPIO_NUM_4;
    cfg.rst  = GPIO_NUM_21;
    cfg.bl   = GPIO_NUM_2;
    cfg.spi_fre = 40 * 1000 * 1000;
    cfg.width   = LCD_WIDTH;
    cfg.height  = LCD_HEIGHT;
    cfg.spin    = 1;                   /* 90° rotation for landscape */
    cfg.done_cb = lv_port_flush_ready;
    cfg.cb_param = &disp_drv;

    ili9341_driver_hw_init(&cfg);
}

esp_err_t lv_port_init(void)
{
    lv_init();
    lcd_init();
    lv_port_disp_init();
    lv_port_tick_init();
    return ESP_OK;
}
