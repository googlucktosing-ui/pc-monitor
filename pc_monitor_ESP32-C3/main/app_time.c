#include "app_time.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "app_config.h"
#include "app_state.h"
#include "app_wifi.h"

static const char *TAG = "time";
static bool s_sntp_started;

static void update_time_state(bool valid)
{
    time_t now;
    struct tm timeinfo;
    char time_text[16];
    char date[16];
    char weekday[16];

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(time_text, sizeof(time_text), "%H:%M", &timeinfo);
    strftime(date, sizeof(date), "%m/%d", &timeinfo);
    strftime(weekday, sizeof(weekday), "%a", &timeinfo);
    app_state_set_time(time_text, date, weekday, valid);
}

static bool sync_time_once(void)
{
    if (!app_wifi_wait_connected(15000)) {
        ESP_LOGW(TAG, "No WiFi for SNTP");
        return false;
    }

    if (!s_sntp_started) {
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_setservername(1, "ntp.aliyun.com");
        esp_sntp_init();
        s_sntp_started = true;
    }

    for (int i = 0; i < 20; i++) {
        time_t now = 0;
        struct tm timeinfo = { 0 };
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_year >= (2024 - 1900)) {
            ESP_LOGI(TAG, "SNTP time synced");
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    ESP_LOGW(TAG, "SNTP sync timeout");
    return false;
}

static void time_task(void *arg)
{
    (void)arg;
    setenv("TZ", APP_TIMEZONE, 1);
    tzset();

    bool valid = sync_time_once();
    TickType_t last_sync = xTaskGetTickCount();

    while (1) {
        if (xTaskGetTickCount() - last_sync > pdMS_TO_TICKS(APP_TIME_SYNC_PERIOD_MS)) {
            valid = sync_time_once() || valid;
            last_sync = xTaskGetTickCount();
        }
        update_time_state(valid);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_time_start(void)
{
    xTaskCreatePinnedToCore(time_task, "time_task", 4096, NULL, 3, NULL, 0);
}
