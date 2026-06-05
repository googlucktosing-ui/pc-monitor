#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>

typedef struct {
    float cpu_usage;
    float gpu_usage;
    float gpu_temp;
    float cpu_temp;
    float mem_used;
    float mem_total;
    float disk_used;
    float disk_total;
    float net_up;
    float net_down;
    char hostname[32];
    char cpu_name[48];
    char gpu_name[48];
    char os_info[48];
    bool pc_connected;
    char date[16];
    char weekday[16];
    char time_text[16];
    bool time_valid;
    int indoor_temp_x10;
    int indoor_hum;
    bool sensor_valid;
    bool wifi_connected;
    int outdoor_temp;
    int theme;
    char weather_text[32];
    char city[32];
    bool weather_valid;
} app_state_t;

void app_state_init(void);
void app_state_get(app_state_t *out);
void app_state_set_pc_data(
    float cpu, float gpu, float gpu_temp, float cpu_temp,
    float mem_used, float mem_total,
    float disk_used, float disk_total,
    float net_up, float net_down,
    const char *hostname, const char *cpu_name,
    const char *gpu_name, const char *os_info,
    bool connected
);
void app_state_set_sensor(int temp_x10, int hum, bool valid);
void app_state_set_wifi(bool connected);
void app_state_set_time(const char *time_text, const char *date, const char *weekday, bool valid);
void app_state_set_weather(const char *city, const char *weather, int temp, bool valid);
void app_state_set_theme(int theme);
#endif
