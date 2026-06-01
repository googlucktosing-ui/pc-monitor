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



/* ���� Screen dimensions ���� */

#define SCR_W 280

#define SCR_H 240



/* ���� Color palette ���� */

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



/* ���� Weather Clock widgets ���� */

static lv_obj_t *s_wc_cont;           /* container for whole weather clock */

static lv_obj_t *s_wc_time;

static lv_obj_t *s_wc_date;

static lv_obj_t *s_wc_wifi_label;

static lv_obj_t *s_wc_indoor_temp;

static lv_obj_t *s_wc_hum;

static lv_obj_t *s_wc_indoor_title;
static lv_obj_t *s_wc_weather;



/* ���� PC Monitor widgets ���� */

static lv_obj_t *s_pc_cont;           /* container for whole PC monitor */

static lv_obj_t *s_pc_time_label;

static lv_obj_t *s_pc_wifi_dot;

static lv_obj_t *s_pc_pc_dot;

static lv_obj_t *s_pc_cpu_val;

static lv_obj_t *s_pc_cpu_bar;

static lv_obj_t *s_pc_gpu_val;

static lv_obj_t *s_pc_gpu_bar;

static lv_obj_t *s_pc_gpu_temp_label;

static lv_obj_t *s_pc_gpu_title;



static lv_obj_t *s_pc_mem_label;

static lv_obj_t *s_pc_mem_bar;

static lv_obj_t *s_pc_disk_label;

static lv_obj_t *s_pc_disk_bar;

static lv_obj_t *s_pc_net_label;

static lv_obj_t *s_pc_bottom_label;





/* �T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T

   ���� Weather Clock UI ����

   �T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T */



static void create_weather_clock_ui(lv_obj_t *parent)

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

    lv_obj_set_size(line, 244, 2);

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



static void refresh_weather_clock(void)

{

    app_state_t st;

    char buf[64];



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





/* �T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T

   ���� PC Monitor UI ����

   �T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T */



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



static void create_pc_monitor_ui(lv_obj_t *parent)

{

    /* ���� Top bar: time ���� */

    s_pc_time_label = lv_label_create(parent);

    lv_label_set_text(s_pc_time_label, "--:--");

    lv_obj_set_style_text_color(s_pc_time_label, CLR_TEXT, 0);

    lv_obj_set_style_text_font(s_pc_time_label, &lv_font_montserrat_28, 0);

    lv_obj_align(s_pc_time_label, LV_ALIGN_TOP_LEFT, 12, 6);



    /* Status dots */

    s_pc_wifi_dot = status_dot_create(parent, 228, 14);

    lv_obj_t *wifi_lbl = lv_label_create(parent);

    lv_label_set_text(wifi_lbl, "WiFi");

    lv_obj_set_style_text_color(wifi_lbl, CLR_MUTED, 0);

    lv_obj_set_style_text_font(wifi_lbl, &lv_font_montserrat_14, 0);

    lv_obj_align_to(wifi_lbl, s_pc_wifi_dot, LV_ALIGN_OUT_RIGHT_MID, 4, 0);



    s_pc_pc_dot = status_dot_create(parent, 228, 30);

    lv_obj_t *pc_lbl = lv_label_create(parent);

    lv_label_set_text(pc_lbl, "PC");

    lv_obj_set_style_text_color(pc_lbl, CLR_MUTED, 0);

    lv_obj_set_style_text_font(pc_lbl, &lv_font_montserrat_14, 0);

    lv_obj_align_to(pc_lbl, s_pc_pc_dot, LV_ALIGN_OUT_RIGHT_MID, 4, 0);



    /* ���� Divider line ���� */

    lv_obj_t *line = lv_obj_create(parent);

    lv_obj_set_size(line, 264, 1);

    lv_obj_set_style_bg_color(line, CLR_LINE, 0);

    lv_obj_set_style_border_width(line, 0, 0);

    lv_obj_set_style_pad_all(line, 0, 0);

    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 42);



    /* ���� CPU Card ���� */

    lv_obj_t *cpu_card = lv_obj_create(parent);

    lv_obj_set_size(cpu_card, 132, 80);

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

    lv_obj_align(s_pc_cpu_val, LV_ALIGN_BOTTOM_RIGHT, -8, -6);



    lv_obj_t *cpu_bar_full = lv_bar_create(cpu_card);

    lv_obj_set_size(cpu_bar_full, 116, 8);

    lv_obj_set_style_bg_color(cpu_bar_full, lv_color_hex(0x334155), 0);

    lv_obj_set_style_bg_color(cpu_bar_full, CLR_CPU, LV_PART_INDICATOR);

    lv_bar_set_range(cpu_bar_full, 0, 100);

    lv_obj_align(cpu_bar_full, LV_ALIGN_BOTTOM_LEFT, 8, -12);

    s_pc_cpu_bar = cpu_bar_full;



    /* ���� GPU Card ���� */

    lv_obj_t *gpu_card = lv_obj_create(parent);

    lv_obj_set_size(gpu_card, 132, 80);

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



    lv_obj_t *gpu_bar_full = lv_bar_create(gpu_card);

    lv_obj_set_size(gpu_bar_full, 116, 8);

    lv_obj_set_style_bg_color(gpu_bar_full, lv_color_hex(0x334155), 0);

    lv_obj_set_style_bg_color(gpu_bar_full, CLR_GPU, LV_PART_INDICATOR);

    lv_bar_set_range(gpu_bar_full, 0, 100);

    lv_obj_align(gpu_bar_full, LV_ALIGN_BOTTOM_LEFT, 8, -12);

    s_pc_gpu_bar = gpu_bar_full;



    /* ���� Memory section ���� */

    s_pc_mem_label = lv_label_create(parent);

    lv_label_set_text(s_pc_mem_label, "Memory  --.-- / --.-- GB");

    lv_obj_set_style_text_color(s_pc_mem_label, CLR_MEM, 0);

    lv_obj_set_style_text_font(s_pc_mem_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_pc_mem_label, LV_ALIGN_TOP_LEFT, 10, 142);



    s_pc_mem_bar = compact_bar_create(parent, 10, 158, 260, CLR_MEM);



    /* ���� Disk section ���� */

    s_pc_disk_label = lv_label_create(parent);

    lv_label_set_text(s_pc_disk_label, "Disk    --.-- / --.-- GB");

    lv_obj_set_style_text_color(s_pc_disk_label, CLR_DISK, 0);

    lv_obj_set_style_text_font(s_pc_disk_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_pc_disk_label, LV_ALIGN_TOP_LEFT, 10, 174);



    s_pc_disk_bar = compact_bar_create(parent, 10, 190, 260, CLR_DISK);



    /* ���� Network ���� */

    s_pc_net_label = lv_label_create(parent);

    lv_label_set_text(s_pc_net_label, "Net  \25\200 -- KB/s  \25\214 -- KB/s");

    lv_obj_set_style_text_color(s_pc_net_label, CLR_MUTED, 0);

    lv_obj_set_style_text_font(s_pc_net_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_pc_net_label, LV_ALIGN_BOTTOM_LEFT, 10, -18);



    /* ���� Bottom hostname ���� */

    s_pc_bottom_label = lv_label_create(parent);

    lv_label_set_text(s_pc_bottom_label, "Discovering PC...");

    lv_obj_set_style_text_color(s_pc_bottom_label, CLR_MUTED, 0);

    lv_obj_set_style_text_font(s_pc_bottom_label, &lv_font_montserrat_14, 0);

    lv_obj_align(s_pc_bottom_label, LV_ALIGN_BOTTOM_RIGHT, -10, -18);

}



static void refresh_pc_monitor(void)

{

    app_state_t st;

    char buf[64];



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
        lv_obj_clear_flag(s_pc_gpu_bar, LV_OBJ_FLAG_HIDDEN);
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
        lv_bar_set_value(s_pc_gpu_bar, (int)(st.gpu_usage + 0.5f), LV_ANIM_OFF);
    } else {
        /* No GPU - show CPU temp prominently */
        lv_obj_clear_flag(s_pc_gpu_title, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_pc_gpu_val, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_pc_gpu_bar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(s_pc_gpu_temp_label, LV_OBJ_FLAG_HIDDEN);
        if (st.cpu_temp > 0) {
            snprintf(buf, sizeof(buf), "%.0f C", st.cpu_temp);
        } else {
            snprintf(buf, sizeof(buf), "--");
        }
        lv_label_set_text(s_pc_gpu_val, buf);
        lv_label_set_text(s_pc_gpu_title, "CPU Temp");
        lv_obj_set_style_text_color(s_pc_gpu_title, lv_color_hex(0x60A5FA), 0);
        lv_bar_set_value(s_pc_gpu_bar, 0, LV_ANIM_OFF);
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

        snprintf(buf, sizeof(buf), "Net  \25\200 %.0f KB/s  \25\214 %.0f KB/s",

                 st.net_up, st.net_down);

    } else {

        snprintf(buf, sizeof(buf), "Net  \25\200 -- KB/s  \25\214 -- KB/s");

    }

    lv_label_set_text(s_pc_net_label, buf);



    /* Bottom: hostname + OS / sensor bonus */

    if (st.pc_connected) {

        snprintf(buf, sizeof(buf), "%s  %s", st.hostname, st.os_info);

    } else if (st.wifi_connected) {

        snprintf(buf, sizeof(buf), "Waiting for PC server...");

    } else {

        snprintf(buf, sizeof(buf), "WiFi connecting...");

    }



    /* Append indoor temp if sensor is valid */

    if (st.sensor_valid) {

        char temp_buf[16];

        snprintf(temp_buf, sizeof(temp_buf), "  |  %d.%dC %d%%",

                 st.indoor_temp_x10 / 10, st.indoor_temp_x10 % 10, st.indoor_hum);

        strncat(buf, temp_buf, sizeof(buf) - strlen(buf) - 1);

    }



    lv_label_set_text(s_pc_bottom_label, buf);

}





/* �T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T

   ���� Mode switching & main UI task ����

   �T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T */



static bool s_show_weather_clock = true;  /* which page is visible */



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

    create_weather_clock_ui(s_wc_cont);



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



    /* Auto-switch mode based on PC connection */

    switch_mode(!st.pc_connected);



    if (s_show_weather_clock) {

        refresh_weather_clock();

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


