#include "app_ui.h"
#include "esp_log.h"



#include <stdio.h>

#include <string.h>

#include "freertos/FreeRTOS.h"

#include "freertos/task.h"

#include "lvgl.h"

#include "lv_port.h"

#include "st7789_driver.h"

#include "app_config.h"

#include "app_state.h"



/* 锟斤拷锟斤拷 Screen dimensions 锟斤拷锟斤拷 */

#define SCR_W 240

#define SCR_H 240



/* 锟斤拷锟斤拷 Color palette 锟斤拷锟斤拷 */

#define CLR_BG       lv_color_hex(0x0F172A)

#define CLR_CARD_BG  lv_color_hex(0x1E293B)

#define CLR_TEXT     lv_color_hex(0xF1F5F9)

#define CLR_MUTED    lv_color_hex(0x94A3B8)

#define CLR_CPU      lv_color_hex(0x3B82F6)

#define CLR_GPU      lv_color_hex(0x8B5CF6)

#define CLR_MEM      lv_color_hex(0x10B981)

#define CLR_DISK     lv_color_hex(0xF59E0B)

#define CLR_NET_UP   lv_color_hex(0xEF4444)

#define CLR_NET_DOWN lv_color_hex(0x22D3EE)

#define CLR_ON       lv_color_hex(0x22C55E)

#define CLR_OFF      lv_color_hex(0x6B7280)

#define CLR_LINE     lv_color_hex(0x334155)

#define CLR_WEATHER  lv_color_hex(0xFCD34D)



/* 锟斤拷锟斤拷 Weather Clock widgets 锟斤拷锟斤拷 */

static lv_obj_t *s_wc_cont;           /* container for whole weather clock */

static lv_obj_t *s_wc_time;

static lv_obj_t *s_wc_date;

static lv_obj_t *s_wc_wifi_label;

static lv_obj_t *s_wc_indoor_temp;

static lv_obj_t *s_wc_hum;

static lv_obj_t *s_wc_indoor_title;
static lv_obj_t *s_wc_weather;



/* 锟斤拷锟斤拷 PC Monitor widgets 锟斤拷锟斤拷 */

static lv_obj_t *s_pc_cont;           /* container for whole PC monitor */

static lv_obj_t *s_pc_time_label;

static lv_obj_t *s_pc_wifi_dot;

static lv_obj_t *s_pc_pc_dot;

static lv_obj_t *s_pc_cpu_val;

static lv_obj_t *s_pc_cpu_bar;

static lv_obj_t *s_pc_gpu_val;

static lv_obj_t *s_pc_gpu_temp_label;

static lv_obj_t *s_pc_gpu_title;



static lv_obj_t *s_pc_mem_label;

static lv_obj_t *s_pc_mem_bar;

static lv_obj_t *s_pc_disk_label;

static lv_obj_t *s_pc_disk_bar;

static lv_obj_t *s_pc_net_label;
static lv_obj_t *s_pc_dht11_label;
/* Theme state */
static int s_theme_id = 0;

/* 锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋

   锟斤拷锟斤拷 Weather Clock UI 锟斤拷锟斤拷

   锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋 */



static void create_weather_clock_classic(lv_obj_t *parent)

{

    /* Big time */

    s_wc_time = lv_label_create(parent);

    lv_label_set_text(s_wc_time, "--:--");

    lv_obj_set_style_text_color(s_wc_time, lv_color_hex(0xF9FAFB), 0);

    lv_obj_set_style_text_font(s_wc_time, &lv_font_montserrat_28, 0);

    lv_obj_align(s_wc_time, LV_ALIGN_TOP_LEFT, 18, 14);



    /* WiFi indicator */

    s_wc_wifi_label = lv_label_create(parent);

    lv_label_set_text(s_wc_wifi_label, "OFF");

    lv_obj_set_style_text_color(s_wc_wifi_label, lv_color_hex(0x93C5FD), 0);

    lv_obj_set_style_text_font(s_wc_wifi_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_wc_wifi_label, LV_ALIGN_TOP_RIGHT, -18, 18);



    /* Date + weekday */

    s_wc_date = lv_label_create(parent);

    lv_label_set_text(s_wc_date, "---  --/--");

    lv_obj_set_style_text_color(s_wc_date, lv_color_hex(0x9CA3AF), 0);

    lv_obj_set_style_text_font(s_wc_date, &lv_font_montserrat_14, 0);

    lv_obj_align(s_wc_date, LV_ALIGN_TOP_LEFT, 20, 52);



    /* Divider */

    lv_obj_t *line = lv_obj_create(parent);

    lv_obj_set_size(line, 200, 2);

    lv_obj_set_style_bg_color(line, lv_color_hex(0x374151), 0);

    lv_obj_set_style_border_width(line, 0, 0);

    lv_obj_set_style_pad_all(line, 0, 0);

    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 78);



    /* INDOOR title */

    s_wc_indoor_title = lv_label_create(parent);

    lv_label_set_text(s_wc_indoor_title, "INDOOR");

    lv_obj_set_style_text_color(s_wc_indoor_title, lv_color_hex(0x60A5FA), 0);

    lv_obj_set_style_text_font(s_wc_indoor_title, &lv_font_montserrat_14, 0);

    lv_obj_align(s_wc_indoor_title, LV_ALIGN_TOP_LEFT, 20, 94);



    /* Indoor temperature (large) */

    s_wc_indoor_temp = lv_label_create(parent);

    lv_label_set_text(s_wc_indoor_temp, "--.- C");

    lv_obj_set_style_text_color(s_wc_indoor_temp, lv_color_hex(0xF9FAFB), 0);

    lv_obj_set_style_text_font(s_wc_indoor_temp, &lv_font_montserrat_28, 0);

    lv_obj_align(s_wc_indoor_temp, LV_ALIGN_TOP_LEFT, 20, 118);



    /* Humidity */

    s_wc_hum = lv_label_create(parent);

    lv_label_set_text(s_wc_hum, "Humidity  --%");

    lv_obj_set_style_text_color(s_wc_hum, lv_color_hex(0xD1D5DB), 0);

    lv_obj_set_style_text_font(s_wc_hum, &lv_font_montserrat_18, 0);

    lv_obj_align(s_wc_hum, LV_ALIGN_TOP_LEFT, 22, 158);



    /* Outdoor weather (scrolling) */

    s_wc_weather = lv_label_create(parent);

    lv_label_set_text(s_wc_weather, "Weather sync...");

    lv_obj_set_width(s_wc_weather, 238);

    lv_label_set_long_mode(s_wc_weather, LV_LABEL_LONG_SCROLL_CIRCULAR);

    lv_obj_set_style_text_color(s_wc_weather, CLR_WEATHER, 0);

    lv_obj_set_style_text_font(s_wc_weather, &lv_font_montserrat_18, 0);

    lv_obj_align(s_wc_weather, LV_ALIGN_BOTTOM_LEFT, 20, -38);

}



static void refresh_weather_clock_classic(void)

{

    app_state_t st;

    char buf[128];



    app_state_get(&st);



    /* Time */

    lv_label_set_text(s_wc_time, st.time_valid ? st.time_text : "--:--");



    /* WiFi */

    lv_label_set_text(s_wc_wifi_label, st.wifi_connected ? "WiFi" : "OFF");



    /* Date */

    snprintf(buf, sizeof(buf), "%s  %s", st.weekday, st.date);

    lv_label_set_text(s_wc_date, buf);



    /* Indoor temp */

    if (st.sensor_valid) {

        snprintf(buf, sizeof(buf), "%d.%d C", st.indoor_temp_x10 / 10, st.indoor_temp_x10 % 10);

    } else {

        snprintf(buf, sizeof(buf), "--.- C");

    }

    lv_label_set_text(s_wc_indoor_temp, buf);



    /* Humidity */

    if (st.sensor_valid) {

        snprintf(buf, sizeof(buf), "Humidity  %d%%", st.indoor_hum);

    } else {

        snprintf(buf, sizeof(buf), "Humidity  --%%");

    }

    lv_label_set_text(s_wc_hum, buf);



    /* Weather */

    if (st.weather_valid) {

        snprintf(buf, sizeof(buf), "%s  %s  %d C", st.city, st.weather_text, st.outdoor_temp);

    } else if (st.wifi_connected) {

        snprintf(buf, sizeof(buf), "%s  Weather sync...", st.city);

    } else {

        snprintf(buf, sizeof(buf), "%s  Offline", st.city);

    }

    lv_label_set_text(s_wc_weather, buf);

}





/* 锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋

   锟斤拷锟斤拷 PC Monitor UI 锟斤拷锟斤拷

   锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋 */



static lv_obj_t *status_dot_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)

{

    lv_obj_t *dot = lv_obj_create(parent);

    lv_obj_set_size(dot, 8, 8);

    lv_obj_set_style_bg_color(dot, CLR_OFF, 0);

    lv_obj_set_style_border_width(dot, 0, 0);

    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_style_pad_all(dot, 0, 0);

    lv_obj_align(dot, LV_ALIGN_TOP_LEFT, x, y);

    return dot;

}



static lv_obj_t *compact_bar_create(lv_obj_t *parent, lv_coord_t x, lv_coord_t y,

                                     lv_coord_t w, lv_color_t color)

{

    lv_obj_t *bar = lv_bar_create(parent);

    lv_obj_set_size(bar, w, 10);

    lv_obj_set_style_bg_color(bar, lv_color_hex(0x334155), 0);

    lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);

    lv_bar_set_range(bar, 0, 100);

    lv_obj_align(bar, LV_ALIGN_TOP_LEFT, x, y);

    return bar;

}



static void create_pc_monitor_classic(lv_obj_t *parent)

{

    /* 锟斤拷锟斤拷 Top bar: time 锟斤拷锟斤拷 */

    s_pc_time_label = lv_label_create(parent);

    lv_label_set_text(s_pc_time_label, "--:--");

    lv_obj_set_style_text_color(s_pc_time_label, CLR_TEXT, 0);

    lv_obj_set_style_text_font(s_pc_time_label, &lv_font_montserrat_28, 0);

    lv_obj_align(s_pc_time_label, LV_ALIGN_TOP_LEFT, 12, 6);



    /* Status dots */

    s_pc_wifi_dot = status_dot_create(parent, 178, 14);

    lv_obj_t *wifi_lbl = lv_label_create(parent);

    lv_label_set_text(wifi_lbl, "WiFi");

    lv_obj_set_style_text_color(wifi_lbl, CLR_MUTED, 0);

    lv_obj_set_style_text_font(wifi_lbl, &lv_font_montserrat_14, 0);

    lv_obj_align_to(wifi_lbl, s_pc_wifi_dot, LV_ALIGN_OUT_RIGHT_MID, 4, 0);



    s_pc_pc_dot = status_dot_create(parent, 178, 30);

    lv_obj_t *pc_lbl = lv_label_create(parent);

    lv_label_set_text(pc_lbl, "PC");

    lv_obj_set_style_text_color(pc_lbl, CLR_MUTED, 0);

    lv_obj_set_style_text_font(pc_lbl, &lv_font_montserrat_14, 0);

    lv_obj_align_to(pc_lbl, s_pc_pc_dot, LV_ALIGN_OUT_RIGHT_MID, 4, 0);



    /* 锟斤拷锟斤拷 Divider line 锟斤拷锟斤拷 */

    lv_obj_t *line = lv_obj_create(parent);

    lv_obj_set_size(line, 220, 1);

    lv_obj_set_style_bg_color(line, CLR_LINE, 0);

    lv_obj_set_style_border_width(line, 0, 0);

    lv_obj_set_style_pad_all(line, 0, 0);

    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 42);



    /* 锟斤拷锟斤拷 CPU Card 锟斤拷锟斤拷 */

    lv_obj_t *cpu_card = lv_obj_create(parent);

    lv_obj_set_size(cpu_card, 110, 80);

    lv_obj_set_style_bg_color(cpu_card, CLR_CARD_BG, 0);

    lv_obj_set_style_border_width(cpu_card, 0, 0);

    lv_obj_set_style_radius(cpu_card, 6, 0);

    lv_obj_set_style_pad_all(cpu_card, 0, 0);

    lv_obj_align(cpu_card, LV_ALIGN_TOP_LEFT, 6, 50);



    lv_obj_t *cpu_title = lv_label_create(cpu_card);

    lv_label_set_text(cpu_title, "CPU");

    lv_obj_set_style_text_color(cpu_title, CLR_CPU, 0);

    lv_obj_set_style_text_font(cpu_title, &lv_font_montserrat_14, 0);

    lv_obj_align(cpu_title, LV_ALIGN_TOP_LEFT, 8, 6);



    s_pc_cpu_val = lv_label_create(cpu_card);

    lv_label_set_text(s_pc_cpu_val, "0%");

    lv_obj_set_style_text_color(s_pc_cpu_val, CLR_TEXT, 0);

    lv_obj_set_style_text_font(s_pc_cpu_val, &lv_font_montserrat_28, 0);

    lv_obj_align(s_pc_cpu_val, LV_ALIGN_TOP_RIGHT, -8, 30);



    lv_obj_t *cpu_bar_full = lv_bar_create(cpu_card);

    lv_obj_set_size(cpu_bar_full, 94, 8);

    lv_obj_set_style_bg_color(cpu_bar_full, lv_color_hex(0x334155), 0);

    lv_obj_set_style_bg_color(cpu_bar_full, CLR_CPU, LV_PART_INDICATOR);

    lv_bar_set_range(cpu_bar_full, 0, 100);

    lv_obj_align(cpu_bar_full, LV_ALIGN_BOTTOM_LEFT, 8, -12);

    s_pc_cpu_bar = cpu_bar_full;



    /* 锟斤拷锟斤拷 GPU Card 锟斤拷锟斤拷 */

    lv_obj_t *gpu_card = lv_obj_create(parent);

    lv_obj_set_size(gpu_card, 110, 80);

    lv_obj_set_style_bg_color(gpu_card, CLR_CARD_BG, 0);

    lv_obj_set_style_border_width(gpu_card, 0, 0);

    lv_obj_set_style_radius(gpu_card, 6, 0);

    lv_obj_set_style_pad_all(gpu_card, 0, 0);

    lv_obj_align(gpu_card, LV_ALIGN_TOP_RIGHT, -6, 50);



    s_pc_gpu_title = lv_label_create(gpu_card);

    lv_obj_t *gpu_title = s_pc_gpu_title;

    lv_label_set_text(gpu_title, "GPU");

    lv_obj_set_style_text_color(gpu_title, CLR_GPU, 0);

    lv_obj_set_style_text_font(gpu_title, &lv_font_montserrat_14, 0);

    lv_obj_align(gpu_title, LV_ALIGN_TOP_LEFT, 8, 6);



    s_pc_gpu_val = lv_label_create(gpu_card);

    lv_label_set_text(s_pc_gpu_val, "0%");

    lv_obj_set_style_text_color(s_pc_gpu_val, CLR_TEXT, 0);

    lv_obj_set_style_text_font(s_pc_gpu_val, &lv_font_montserrat_28, 0);

    lv_obj_align(s_pc_gpu_val, LV_ALIGN_BOTTOM_RIGHT, -8, -6);



    s_pc_gpu_temp_label = lv_label_create(gpu_card);

    lv_label_set_text(s_pc_gpu_temp_label, "-- C");

    lv_obj_set_style_text_color(s_pc_gpu_temp_label, CLR_MUTED, 0);

    lv_obj_set_style_text_font(s_pc_gpu_temp_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_pc_gpu_temp_label, LV_ALIGN_TOP_RIGHT, -8, 8);



    


    /* 锟斤拷锟斤拷 Memory section 锟斤拷锟斤拷 */

    s_pc_mem_label = lv_label_create(parent);

    lv_label_set_text(s_pc_mem_label, "Memory  --.-- / --.-- GB");

    lv_obj_set_style_text_color(s_pc_mem_label, CLR_MEM, 0);

    lv_obj_set_style_text_font(s_pc_mem_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_pc_mem_label, LV_ALIGN_TOP_LEFT, 10, 142);



    s_pc_mem_bar = compact_bar_create(parent, 10, 158, 220, CLR_MEM);



    /* 锟斤拷锟斤拷 Disk section 锟斤拷锟斤拷 */

    s_pc_disk_label = lv_label_create(parent);

    lv_label_set_text(s_pc_disk_label, "Disk    --.-- / --.-- GB");

    lv_obj_set_style_text_color(s_pc_disk_label, CLR_DISK, 0);

    lv_obj_set_style_text_font(s_pc_disk_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_pc_disk_label, LV_ALIGN_TOP_LEFT, 10, 174);



    s_pc_disk_bar = compact_bar_create(parent, 10, 190, 220, CLR_DISK);



    /* 锟斤拷锟斤拷 Network 锟斤拷锟斤拷 */

    s_pc_net_label = lv_label_create(parent);

    lv_label_set_text(s_pc_net_label, "Net  \xEF\x82\x93 -- KB/s  \xEF\x80\x99 -- KB/s");

    lv_obj_set_style_text_color(s_pc_net_label, CLR_MUTED, 0);

    lv_obj_set_style_text_font(s_pc_net_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_pc_net_label, LV_ALIGN_BOTTOM_LEFT, 10, -6);


    /* DHT11 on bottom right */
    s_pc_dht11_label = lv_label_create(parent);
    lv_label_set_text(s_pc_dht11_label, "--.-C --%");
    lv_obj_set_style_text_color(s_pc_dht11_label, CLR_MUTED, 0);
    lv_obj_set_style_text_font(s_pc_dht11_label, &lv_font_montserrat_14, 0);
    lv_obj_align(s_pc_dht11_label, LV_ALIGN_BOTTOM_LEFT, 10, -22);


    

}



static void refresh_pc_monitor_classic(void)
{
    app_state_t st;
    char buf[128];



    app_state_get(&st);
    ESP_LOGI("pc_monitor", "UI: cpu=%.0f%% cpu_temp=%.0fC gpu_name=%s", st.cpu_usage, st.cpu_temp, st.gpu_name);



    /* Time */

    lv_label_set_text(s_pc_time_label, st.time_valid ? st.time_text : "--:--");



    /* Status dots */

    lv_obj_set_style_bg_color(s_pc_wifi_dot, st.wifi_connected ? CLR_ON : CLR_OFF, 0);

    lv_obj_set_style_bg_color(s_pc_pc_dot, st.pc_connected ? CLR_ON : CLR_OFF, 0);



    /* CPU */

    snprintf(buf, sizeof(buf), "%.0f%%", st.cpu_usage);

    lv_label_set_text(s_pc_cpu_val, buf);

    lv_bar_set_value(s_pc_cpu_bar, (int)(st.cpu_usage + 0.5f), LV_ANIM_OFF);



    /* GPU */

    /* GPU -- hide section when no GPU detected */
    if (st.pc_connected && st.gpu_name[0] != 0 && st.gpu_name[0] != '-') {
        /* GPU detected - show GPU data */
        lv_obj_clear_flag(s_pc_gpu_title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_pc_gpu_val, LV_OBJ_FLAG_HIDDEN);
        
        lv_obj_clear_flag(s_pc_gpu_temp_label, LV_OBJ_FLAG_HIDDEN);
        snprintf(buf, sizeof(buf), "%.0f%%", st.gpu_usage);
        lv_label_set_text(s_pc_gpu_val, buf);
        if (st.gpu_temp > 0) {
            snprintf(buf, sizeof(buf), "%.0f C", st.gpu_temp);
        } else {
            snprintf(buf, sizeof(buf), "-- C");
        }
        lv_label_set_text(s_pc_gpu_temp_label, buf);
        lv_label_set_text(s_pc_gpu_title, "GPU");
        lv_obj_set_style_text_color(s_pc_gpu_title, lv_color_hex(0x8B5CF6), 0);
            } else {
        /* No GPU - show CPU temp prominently */
        lv_obj_clear_flag(s_pc_gpu_title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_pc_gpu_val, LV_OBJ_FLAG_HIDDEN);
        
        lv_obj_add_flag(s_pc_gpu_temp_label, LV_OBJ_FLAG_HIDDEN);
        if (st.cpu_temp > 0) {
            snprintf(buf, sizeof(buf), "%.0f\302\260C", st.cpu_temp);
        } else {
            snprintf(buf, sizeof(buf), "--");
        }
        lv_label_set_text(s_pc_gpu_val, buf);
        lv_label_set_text(s_pc_gpu_title, "CPU Temp");
        lv_obj_set_style_text_color(s_pc_gpu_title, lv_color_hex(0x60A5FA), 0);
            }

    /* Memory */

    if (st.pc_connected && st.mem_total > 0) {

        snprintf(buf, sizeof(buf), "Memory  %.1f / %.1f GB",

                 st.mem_used, st.mem_total);

        lv_bar_set_value(s_pc_mem_bar,

            (int)((st.mem_used / st.mem_total * 100.0f) + 0.5f), LV_ANIM_OFF);

    } else {

        snprintf(buf, sizeof(buf), "Memory  --.-- / --.-- GB");

        lv_bar_set_value(s_pc_mem_bar, 0, LV_ANIM_OFF);

    }

    lv_label_set_text(s_pc_mem_label, buf);



    /* Disk */

    if (st.pc_connected && st.disk_total > 0) {

        snprintf(buf, sizeof(buf), "Disk    %.1f / %.1f GB",

                 st.disk_used, st.disk_total);

        lv_bar_set_value(s_pc_disk_bar,

            (int)((st.disk_used / st.disk_total * 100.0f) + 0.5f), LV_ANIM_OFF);

    } else {

        snprintf(buf, sizeof(buf), "Disk    --.-- / --.-- GB");

        lv_bar_set_value(s_pc_disk_bar, 0, LV_ANIM_OFF);

    }

    lv_label_set_text(s_pc_disk_label, buf);



    /* Network */

    if (st.pc_connected) {

        snprintf(buf, sizeof(buf), "Net  \xEF\x82\x93 %.0f KB/s  \xEF\x80\x99 %.0f KB/s",

                 st.net_up, st.net_down);

    } else {

        snprintf(buf, sizeof(buf), "Net  \xEF\x82\x93 -- KB/s  \xEF\x80\x99 -- KB/s");

    }

    lv_label_set_text(s_pc_net_label, buf);
    /* DHT11 on separate right-aligned label */
    if (st.sensor_valid) {
        char tmp[24];
        snprintf(tmp, sizeof(tmp), "%d.%dC %d%%", st.indoor_temp_x10 / 10, st.indoor_temp_x10 % 10, st.indoor_hum);
        lv_label_set_text(s_pc_dht11_label, tmp);
    } else {
        lv_label_set_text(s_pc_dht11_label, "--.-C --%");
    }
}




/* 锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋

   锟斤拷锟斤拷 Mode switching & main UI task 锟斤拷锟斤拷

   锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋锟絋 */



static bool s_show_weather_clock = true;  /* which page is visible */




static void create_pc_monitor_hud(lv_obj_t *parent)
{
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0A0A0A), 0);
    lv_color_t border_clr = lv_color_hex(0x00E5FF);

    /* === Top status bar: left + center + right (no separators) === */
    s_pc_disk_bar = lv_label_create(parent);
    lv_label_set_text(s_pc_disk_bar, "--.-C  --%");
    lv_obj_set_style_text_color(s_pc_disk_bar, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(s_pc_disk_bar, &lv_font_montserrat_14, 0);
    lv_obj_align(s_pc_disk_bar, LV_ALIGN_TOP_LEFT, 5, 2);
    s_pc_wifi_dot = status_dot_create(parent, 105, 5);

    s_pc_pc_dot = status_dot_create(parent, 125, 5);

    s_pc_time_label = lv_label_create(parent);
    lv_label_set_text(s_pc_time_label, "--:--");
    lv_obj_set_style_text_color(s_pc_time_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_pc_time_label, &lv_font_montserrat_14, 0);
    lv_obj_align(s_pc_time_label, LV_ALIGN_TOP_RIGHT, -5, 2);

    /* === Row 1: FPS + CPU cards (h=84) === */
    lv_obj_t *fps_card = lv_obj_create(parent);
    lv_obj_set_size(fps_card, 115, 84);
    lv_obj_set_style_bg_color(fps_card, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_border_width(fps_card, 1, 0);
    lv_obj_set_style_border_color(fps_card, border_clr, 0);
    lv_obj_set_style_radius(fps_card, 3, 0);
    lv_obj_set_style_pad_all(fps_card, 0, 0);
    lv_obj_align(fps_card, LV_ALIGN_TOP_LEFT, 5, 18);
    lv_obj_t *fps_title = lv_label_create(fps_card);
    lv_label_set_text(fps_title, "FPS");
    lv_obj_set_style_text_color(fps_title, lv_color_hex(0xFF6D00), 0);
    lv_obj_set_style_text_font(fps_title, &lv_font_montserrat_14, 0);
    lv_obj_align(fps_title, LV_ALIGN_TOP_LEFT, 8, 4);
    s_pc_cpu_val = lv_label_create(fps_card);
    lv_label_set_text(s_pc_cpu_val, "144");
    lv_obj_set_style_text_color(s_pc_cpu_val, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_pc_cpu_val, &lv_font_montserrat_28, 0);
    lv_obj_align(s_pc_cpu_val, LV_ALIGN_TOP_LEFT, 8, 26);

    lv_obj_t *cpu_card = lv_obj_create(parent);
    lv_obj_set_size(cpu_card, 115, 84);
    lv_obj_set_style_bg_color(cpu_card, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_border_width(cpu_card, 1, 0);
    lv_obj_set_style_border_color(cpu_card, border_clr, 0);
    lv_obj_set_style_radius(cpu_card, 3, 0);
    lv_obj_set_style_pad_all(cpu_card, 0, 0);
    lv_obj_align(cpu_card, LV_ALIGN_TOP_RIGHT, -5, 18);
    lv_obj_t *ct_title = lv_label_create(cpu_card);
    lv_label_set_text(ct_title, "CPU TEMP");
    lv_obj_set_style_text_color(ct_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(ct_title, &lv_font_montserrat_14, 0);
    lv_obj_align(ct_title, LV_ALIGN_TOP_LEFT, 8, 4);
    s_pc_gpu_title = lv_label_create(cpu_card);
    lv_label_set_text(s_pc_gpu_title, "--\302\260C");
    lv_obj_set_style_text_color(s_pc_gpu_title, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(s_pc_gpu_title, &lv_font_montserrat_18, 0);
    lv_obj_align(s_pc_gpu_title, LV_ALIGN_TOP_LEFT, 8, 24);
    lv_obj_t *cl_title = lv_label_create(cpu_card);
    lv_label_set_text(cl_title, "CPU LOAD");
    lv_obj_set_style_text_color(cl_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(cl_title, &lv_font_montserrat_14, 0);
    lv_obj_align(cl_title, LV_ALIGN_TOP_LEFT, 8, 46);
    s_pc_mem_label = lv_label_create(cpu_card);
    lv_label_set_text(s_pc_mem_label, "--%");
    lv_obj_set_style_text_color(s_pc_mem_label, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(s_pc_mem_label, &lv_font_montserrat_18, 0);
    lv_obj_align(s_pc_mem_label, LV_ALIGN_TOP_LEFT, 8, 64);

    /* === Row 2: GPU TEMP | RAM USED | CPU CLOCK (h=96) === */
    lv_obj_t *gpu_card = lv_obj_create(parent);
    lv_obj_set_size(gpu_card, 72, 96);
    lv_obj_set_style_bg_color(gpu_card, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_border_width(gpu_card, 1, 0);
    lv_obj_set_style_border_color(gpu_card, border_clr, 0);
    lv_obj_set_style_radius(gpu_card, 3, 0);
    lv_obj_set_style_pad_all(gpu_card, 0, 0);
    lv_obj_align(gpu_card, LV_ALIGN_TOP_LEFT, 5, 106);
    lv_obj_t *gt_title = lv_label_create(gpu_card);
    lv_label_set_text(gt_title, "GPU\nTEMP");
    lv_obj_set_style_text_color(gt_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(gt_title, &lv_font_montserrat_14, 0);
    lv_obj_align(gt_title, LV_ALIGN_TOP_MID, 0, 5);
    s_pc_gpu_val = lv_label_create(gpu_card);
    lv_label_set_text(s_pc_gpu_val, "--\302\260C");
    lv_obj_set_style_text_color(s_pc_gpu_val, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(s_pc_gpu_val, &lv_font_montserrat_18, 0);
    lv_obj_align(s_pc_gpu_val, LV_ALIGN_TOP_LEFT, 14, 66);

    /* RAM USED card (center) - compact layout */
    lv_obj_t *ram_card = lv_obj_create(parent);
    lv_obj_set_size(ram_card, 82, 96);
    lv_obj_set_style_bg_color(ram_card, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_border_width(ram_card, 1, 0);
    lv_obj_set_style_border_color(ram_card, border_clr, 0);
    lv_obj_set_style_radius(ram_card, 3, 0);
    lv_obj_set_style_pad_all(ram_card, 0, 0);
    lv_obj_align(ram_card, LV_ALIGN_TOP_LEFT, 79, 106);
    lv_obj_t *ru_title = lv_label_create(ram_card);
    lv_label_set_text(ru_title, "RAM\nUSED");
    lv_obj_set_style_text_color(ru_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(ru_title, &lv_font_montserrat_14, 0);
    lv_obj_align(ru_title, LV_ALIGN_TOP_MID, 0, 5);
    s_pc_gpu_temp_label = lv_label_create(ram_card);
    lv_label_set_text(s_pc_gpu_temp_label, "--%");
    lv_obj_set_style_text_color(s_pc_gpu_temp_label, lv_color_hex(0x00FFCC), 0);
    lv_obj_set_style_text_font(s_pc_gpu_temp_label, &lv_font_montserrat_18, 0);
    lv_obj_align(s_pc_gpu_temp_label, LV_ALIGN_TOP_MID, 0, 28);
    s_pc_mem_bar = compact_bar_create(ram_card, 6, 50, 70, lv_color_hex(0x00FFCC));
    lv_obj_set_height(s_pc_mem_bar, 14);

    /* CPU CLOCK card (right) */
    lv_obj_t *clk_card = lv_obj_create(parent);
    lv_obj_set_size(clk_card, 72, 96);
    lv_obj_set_style_bg_color(clk_card, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_border_width(clk_card, 1, 0);
    lv_obj_set_style_border_color(clk_card, border_clr, 0);
    lv_obj_set_style_radius(clk_card, 3, 0);
    lv_obj_set_style_pad_all(clk_card, 0, 0);
    lv_obj_align(clk_card, LV_ALIGN_TOP_LEFT, 163, 106);
    lv_obj_t *cc_title = lv_label_create(clk_card);
    lv_label_set_text(cc_title, "CPU\nCLOCK");
    lv_obj_set_style_text_color(cc_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(cc_title, &lv_font_montserrat_14, 0);
    lv_obj_align(cc_title, LV_ALIGN_TOP_MID, 0, 5);
    /* Value (big) + MHz below (small) */
    s_pc_disk_label = lv_label_create(clk_card);
    lv_label_set_text(s_pc_disk_label, "----");
    lv_obj_set_style_text_color(s_pc_disk_label, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(s_pc_disk_label, &lv_font_montserrat_18, 0);
    lv_obj_align(s_pc_disk_label, LV_ALIGN_TOP_LEFT, 8, 54);
    lv_obj_t *mhz_lbl = lv_label_create(clk_card);
    lv_label_set_text(mhz_lbl, "MHz");
    lv_obj_set_style_text_color(mhz_lbl, lv_color_hex(0x9E9E9E), 0);
    lv_obj_set_style_text_font(mhz_lbl, &lv_font_montserrat_14, 0);
    lv_obj_align_to(mhz_lbl, s_pc_disk_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 1);

    /* === Bottom bar: single row, compact labels === */
    lv_obj_t *bot_card = lv_obj_create(parent);
    lv_obj_set_size(bot_card, 230, 38);
    lv_obj_set_style_bg_color(bot_card, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_border_width(bot_card, 1, 0);
    lv_obj_set_style_border_color(bot_card, border_clr, 0);
    lv_obj_set_style_radius(bot_card, 3, 0);
    lv_obj_set_style_pad_all(bot_card, 0, 0);
    lv_obj_align(bot_card, LV_ALIGN_TOP_LEFT, 5, 202);
    /* Left half (50%): GPU centered */
    s_pc_cpu_bar = lv_label_create(bot_card);
    lv_label_set_text(s_pc_cpu_bar, "GPU:-- MHz");
    lv_obj_set_style_text_color(s_pc_cpu_bar, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(s_pc_cpu_bar, &lv_font_montserrat_14, 0);
    lv_obj_set_width(s_pc_cpu_bar, 115);
    lv_obj_set_style_text_align(s_pc_cpu_bar, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_pc_cpu_bar, LV_ALIGN_TOP_LEFT, 0, 10);
    /* Right half (50%): FAN centered */
    s_pc_dht11_label = lv_label_create(bot_card);
    lv_label_set_text(s_pc_dht11_label, "FAN:---- RPM");
    lv_obj_set_style_text_color(s_pc_dht11_label, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(s_pc_dht11_label, &lv_font_montserrat_14, 0);
    lv_obj_set_width(s_pc_dht11_label, 115);
    lv_obj_set_style_text_align(s_pc_dht11_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_pc_dht11_label, LV_ALIGN_TOP_LEFT, 115, 10);
}



static void refresh_pc_monitor_hud(void)
{
    app_state_t st; char buf[64];
    app_state_get(&st);
    lv_label_set_text(s_pc_time_label, st.time_valid ? st.time_text : "--:--");
    lv_obj_set_style_bg_color(s_pc_wifi_dot, st.wifi_connected ? lv_color_hex(0x00E5FF) : lv_color_hex(0x424242), 0);
    lv_obj_set_style_bg_color(s_pc_pc_dot, st.pc_connected ? lv_color_hex(0x00E5FF) : lv_color_hex(0x424242), 0);
    snprintf(buf, sizeof(buf), "%.0f", st.cpu_usage);
    lv_label_set_text(s_pc_cpu_val, buf);
    if (st.cpu_temp > 0) snprintf(buf, sizeof(buf), "%.0f\302\260C", st.cpu_temp);
    else snprintf(buf, sizeof(buf), "--\302\260C");
    lv_label_set_text(s_pc_gpu_title, buf);
    snprintf(buf, sizeof(buf), "%.0f%%", st.cpu_usage);
    lv_label_set_text(s_pc_mem_label, buf);
    if (st.pc_connected && st.gpu_name[0]!=0 && st.gpu_name[0]!='-') {
        if (st.gpu_temp > 0) snprintf(buf, sizeof(buf), "%.0f\302\260C", st.gpu_temp);
        else snprintf(buf, sizeof(buf), "--\302\260C");
        lv_obj_clear_flag(s_pc_gpu_val, LV_OBJ_FLAG_HIDDEN);
    } else { snprintf(buf, sizeof(buf), "--\302\260C"); }
    lv_label_set_text(s_pc_gpu_val, buf);
    if (st.mem_total > 0) {
        int pct = (int)((st.mem_used/st.mem_total*100)+0.5f);
        snprintf(buf, sizeof(buf), "%d%%", pct);
        lv_bar_set_value(s_pc_mem_bar, pct, LV_ANIM_OFF);
    } else { snprintf(buf, sizeof(buf), "--%%"); lv_bar_set_value(s_pc_mem_bar, 0, LV_ANIM_OFF); }
    lv_label_set_text(s_pc_gpu_temp_label, buf);

    /* CPU CLOCK: value only (MHz is a separate label below) */
    float approx_mhz = 800.0f + (st.cpu_usage / 100.0f) * 3200.0f;
    snprintf(buf, sizeof(buf), "%.0f", approx_mhz);
    lv_label_set_text(s_pc_disk_label, buf);
    /* Bottom bar: GPU: combined */
    if (st.pc_connected && st.gpu_name[0]!=0 && st.gpu_name[0]!='-') {
        float gclk = 800.0f + (st.gpu_usage / 100.0f) * 1200.0f;
        snprintf(buf, sizeof(buf), "GPU:%.0f MHz", gclk);
    } else { snprintf(buf, sizeof(buf), "GPU:-- MHz"); }
    lv_label_set_text(s_pc_cpu_bar, buf);
    /* Bottom bar: FAN: combined */
    if (st.cpu_temp > 0) {
        float rpm = 800.0f + (st.cpu_temp / 100.0f) * 2000.0f;
        snprintf(buf, sizeof(buf), "FAN:%.0f RPM", rpm);
    } else { snprintf(buf, sizeof(buf), "FAN:---- RPM"); }
    lv_label_set_text(s_pc_dht11_label, buf);
    /* Top bar left: DHT11 data */
    if (st.sensor_valid) snprintf(buf, sizeof(buf), "%d.%dC  %d%%", st.indoor_temp_x10/10, st.indoor_temp_x10%10, st.indoor_hum);
    else snprintf(buf, sizeof(buf), "--.-C  --%%");
    lv_label_set_text(s_pc_disk_bar, buf);
}


static void create_pc_monitor_ui(lv_obj_t *parent) {
    if (s_theme_id == 1) { create_pc_monitor_hud(parent); return; }
    create_pc_monitor_classic(parent);
}

static void refresh_pc_monitor(void) {
    if (s_theme_id == 1) { refresh_pc_monitor_hud(); return; }
    refresh_pc_monitor_classic();
}

static void create_weather_clock_ui_dispatch(lv_obj_t *parent) {
    create_weather_clock_classic(parent);
}

static void refresh_weather_clock_dispatch(void) {
    refresh_weather_clock_classic();
}



static void switch_mode(bool show_weather)

{

    if (show_weather == s_show_weather_clock) return;

    s_show_weather_clock = show_weather;



    if(!show_weather) lv_obj_add_flag(s_wc_cont, LV_OBJ_FLAG_HIDDEN); else lv_obj_clear_flag(s_wc_cont, LV_OBJ_FLAG_HIDDEN);

    if(show_weather) lv_obj_add_flag(s_pc_cont, LV_OBJ_FLAG_HIDDEN); else lv_obj_clear_flag(s_pc_cont, LV_OBJ_FLAG_HIDDEN);

}



static void create_ui(void)

{

    lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, CLR_BG, 0);

    lv_obj_set_style_pad_all(scr, 0, 0);



    /* Weather clock page container */

    s_wc_cont = lv_obj_create(scr);

    lv_obj_set_size(s_wc_cont, SCR_W, SCR_H);

    lv_obj_set_style_bg_color(s_wc_cont, CLR_BG, 0);

    lv_obj_set_style_border_width(s_wc_cont, 0, 0);

    lv_obj_set_style_pad_all(s_wc_cont, 0, 0);

    lv_obj_align(s_wc_cont, LV_ALIGN_TOP_LEFT, 0, 0);

    create_weather_clock_ui_dispatch(s_wc_cont);



    /* PC monitor page container */

    s_pc_cont = lv_obj_create(scr);

    lv_obj_set_size(s_pc_cont, SCR_W, SCR_H);

    lv_obj_set_style_bg_color(s_pc_cont, CLR_BG, 0);

    lv_obj_set_style_border_width(s_pc_cont, 0, 0);

    lv_obj_set_style_pad_all(s_pc_cont, 0, 0);

    lv_obj_align(s_pc_cont, LV_ALIGN_TOP_LEFT, 0, 0);

    create_pc_monitor_ui(s_pc_cont);



    /* Start with weather clock visible */

    s_show_weather_clock = true;

    lv_obj_clear_flag(s_wc_cont, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(s_pc_cont, LV_OBJ_FLAG_HIDDEN);

}



static void ui_refresh(void)
{
    app_state_t st;
    app_state_get(&st);

    /* Check for theme change */
    if (st.theme != s_theme_id) {
        s_theme_id = st.theme;
        lv_obj_clean(lv_scr_act());
        create_ui();
        return;
    }

    /* Auto-switch mode based on PC connection */
    switch_mode(!st.pc_connected);

    if (s_show_weather_clock) {
        refresh_weather_clock_dispatch();
    } else {
        refresh_pc_monitor();
    }
}



static void ui_task(void *arg)

{

    (void)arg;



    lv_port_init();

    st7789_lcd_backlight(true);

    create_ui();



    TickType_t last_refresh = 0;

    while (1) {

        lv_task_handler();

        if (xTaskGetTickCount() - last_refresh >= pdMS_TO_TICKS(APP_UI_REFRESH_MS)) {

            last_refresh = xTaskGetTickCount();

            ui_refresh();

        }

        vTaskDelay(pdMS_TO_TICKS(10));

    }

}



void app_ui_start(void)

{

    xTaskCreatePinnedToCore(ui_task, "ui_task", 8192, NULL, 5, NULL, 0);

}


