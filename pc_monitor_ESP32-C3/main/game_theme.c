#include "game_theme.h"
#include "esp_log.h"
#include "app_state.h"
#include <stdio.h>
#include <string.h>

/* SquareLine exported UI */
#include "game_theme/ui.h"

static const char *TAG = "GAME_THEME";

/* Theme instance */
static bool s_created = false;

/* Initialize game theme as a separate screen */
void game_theme_create(void)
{
    if (s_created) return;
    
    /* SquareLine ui_init() creates Screen1 and loads it */
    ui_init();
    
    s_created = true;
    ESP_LOGI(TAG, "Game theme screen created");
}

/* Destroy game theme screen */
void game_theme_destroy(void)
{
    if (!s_created) return;
    ui_destroy();
    s_created = false;
    ESP_LOGI(TAG, "Game theme screen destroyed");
}

/* Switch to game theme screen */
void game_theme_show(void)
{
    if (!s_created) {
        game_theme_create();
    }
    lv_disp_load_scr(ui_Screen1);
}

/* Update game theme with live data */
void game_theme_refresh(void)
{
    if (!s_created) return;
    
    app_state_t st;
    app_state_get(&st);
    
    char buf[32];
    
    /* CPU temp */
    if (st.pc_connected && st.cpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%.0f°", st.cpu_temp);
    } else {
        snprintf(buf, sizeof(buf), "--°");
    }
    lv_label_set_text(ui_cputempval, buf);
    
    /* GPU temp */
    if (st.pc_connected && st.gpu_temp > 0) {
        snprintf(buf, sizeof(buf), "%.0f°", st.gpu_temp);
    } else {
        snprintf(buf, sizeof(buf), "--°");
    }
    lv_label_set_text(ui_gputempval, buf);
    
    /* CPU usage % and frequency */
    if (st.pc_connected) {
        snprintf(buf, sizeof(buf), "%d%%", (int)st.cpu_usage);
        lv_label_set_text(ui_perfval, buf);
        lv_label_set_text(ui_cpupct, buf);
        lv_arc_set_value(ui_cpu_arc, (int)st.cpu_usage);
        
        float freq = 800.0f + (st.cpu_usage / 100.0f) * 3200.0f;
        snprintf(buf, sizeof(buf), "%.1f GHz", freq / 1000.0f);
        lv_label_set_text(ui_cpufreq, buf);
    } else {
        lv_label_set_text(ui_perfval, "--%");
        lv_label_set_text(ui_cpupct, "--%");
        lv_arc_set_value(ui_cpu_arc, 0);
        lv_label_set_text(ui_cpufreq, "-- GHz");
    }
    
    /* GPU usage % and frequency */
    if (st.pc_connected && st.gpu_usage > 0) {
        snprintf(buf, sizeof(buf), "%d%%", (int)st.gpu_usage);
        lv_label_set_text(ui_gpupct, buf);
        lv_arc_set_value(ui_gpu_arc, (int)st.gpu_usage);
        
        float gclk = 800.0f + (st.gpu_usage / 100.0f) * 1200.0f;
        snprintf(buf, sizeof(buf), "%.1f GHz", gclk / 1000.0f);
        lv_label_set_text(ui_gpufreq, buf);
    } else {
        lv_label_set_text(ui_gpupct, "--%");
        lv_arc_set_value(ui_gpu_arc, 0);
        lv_label_set_text(ui_gpufreq, "-- GHz");
    }
    
    /* RAM */
    if (st.mem_total > 0) {
        int pct = (int)((st.mem_used / st.mem_total * 100) + 0.5f);
        snprintf(buf, sizeof(buf), "%.1f/%.1f GB", st.mem_used, st.mem_total);
        lv_label_set_text(ui_ramlabel, buf);
        lv_bar_set_value(ui_rambar, pct, LV_ANIM_OFF);
    } else {
        lv_label_set_text(ui_ramlabel, "--/-- GB");
        lv_bar_set_value(ui_rambar, 0, LV_ANIM_OFF);
    }
    
    /* Network */
    if (st.pc_connected) {
        snprintf(buf, sizeof(buf), LV_SYMBOL_DOWNLOAD " %.1f MB/s", st.net_down);
        lv_label_set_text(ui_netdown, buf);
        snprintf(buf, sizeof(buf), LV_SYMBOL_UPLOAD " %.1f MB/s", st.net_up);
        lv_label_set_text(ui_netup, buf);
    } else {
        lv_label_set_text(ui_netdown, LV_SYMBOL_DOWNLOAD " -- MB/s");
        lv_label_set_text(ui_netup, LV_SYMBOL_UPLOAD " -- MB/s");
    }
    
    /* Status */
    if (st.pc_connected) {
        lv_label_set_text(ui_statusbar, "PC CONNECTED");
    } else {
        lv_label_set_text(ui_statusbar, "WAITING...");
    }
    
    if (st.sensor_valid) {
        snprintf(buf, sizeof(buf), "%d.%d°C %d%%", 
                 st.indoor_temp_x10 / 10, st.indoor_temp_x10 % 10, st.indoor_hum);
        lv_label_set_text(ui_statusmid, buf);
    } else {
        lv_label_set_text(ui_statusmid, "--.-°C --%");
    }
}

