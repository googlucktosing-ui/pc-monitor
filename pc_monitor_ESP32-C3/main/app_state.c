#include "app_state.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static app_state_t s_state = {0};
static SemaphoreHandle_t s_mutex = NULL;

void app_state_init(void)
{
    s_mutex = xSemaphoreCreateMutex();
    s_state.pc_connected = false;
    s_state.wifi_connected = false;
    s_state.sensor_valid = false;
    s_state.time_valid = false;
    s_state.weather_valid = false;
    strcpy(s_state.hostname, "PC");
    strcpy(s_state.cpu_name, "---");
    strcpy(s_state.gpu_name, "---");
    strcpy(s_state.os_info, "---");
    strcpy(s_state.city, "CITY");
    strcpy(s_state.weather_text, "--");
}

void app_state_get(app_state_t *out)
{
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100))) {
        memcpy(out, &s_state, sizeof(app_state_t));
        xSemaphoreGive(s_mutex);
    }
}

void app_state_set_pc_data(
    float cpu, float gpu, float gpu_temp, float cpu_temp,
    float mem_used, float mem_total,
    float disk_used, float disk_total,
    float net_up, float net_down,
    const char *hostname, const char *cpu_name,
    const char *gpu_name, const char *os_info,
    bool connected)
{
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100))) {
        s_state.cpu_usage = cpu;
        s_state.gpu_usage = gpu;
        s_state.gpu_temp = gpu_temp;
        s_state.cpu_temp = cpu_temp;
        s_state.mem_used = mem_used;
        s_state.mem_total = mem_total;
        s_state.disk_used = disk_used;
        s_state.disk_total = disk_total;
        s_state.net_up = net_up;
        s_state.net_down = net_down;
        if (hostname) strncpy(s_state.hostname, hostname, sizeof(s_state.hostname) - 1);
        if (cpu_name)  strncpy(s_state.cpu_name, cpu_name, sizeof(s_state.cpu_name) - 1);
        if (gpu_name)  strncpy(s_state.gpu_name, gpu_name, sizeof(s_state.gpu_name) - 1);
        if (os_info)   strncpy(s_state.os_info, os_info, sizeof(s_state.os_info) - 1);
        s_state.pc_connected = connected;
        xSemaphoreGive(s_mutex);
    }
}

void app_state_set_sensor(int temp_x10, int hum, bool valid)
{
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100))) {
        s_state.indoor_temp_x10 = temp_x10;
        s_state.indoor_hum = hum;
        s_state.sensor_valid = valid;
        xSemaphoreGive(s_mutex);
    }
}

void app_state_set_wifi(bool connected)
{
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100))) {
        s_state.wifi_connected = connected;
        xSemaphoreGive(s_mutex);
    }
}

void app_state_set_time(const char *time_text, const char *date, const char *weekday, bool valid)
{
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100))) {
        if (time_text) strncpy(s_state.time_text, time_text, sizeof(s_state.time_text) - 1);
        if (date)      strncpy(s_state.date, date, sizeof(s_state.date) - 1);
        if (weekday)   strncpy(s_state.weekday, weekday, sizeof(s_state.weekday) - 1);
        s_state.time_valid = valid;
        xSemaphoreGive(s_mutex);
    }
}

void app_state_set_weather(const char *city, const char *weather, int temp, bool valid)
{
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100))) {
        if (city)    strncpy(s_state.city, city, sizeof(s_state.city) - 1);
        if (weather) strncpy(s_state.weather_text, weather, sizeof(s_state.weather_text) - 1);
        s_state.outdoor_temp = temp;
        s_state.weather_valid = valid;
        xSemaphoreGive(s_mutex);
    }
}


void app_state_set_theme(int theme)
{
    if (theme < 0) theme = 0;
    if (theme != 0) theme = 2;
    s_state.theme = theme;
}
