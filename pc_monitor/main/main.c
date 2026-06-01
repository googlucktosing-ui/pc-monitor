#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "app_state.h"
#include "app_ui.h"
#include "app_sensor.h"
#include "app_wifi.h"
#include "app_time.h"
#include "app_weather.h"
#include "app_discovery.h"
#include "app_pc_monitor.h"
#include "app_serial_cfg.h"

static const char *TAG = "pc_monitor";

static void init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(ret);
    }
}

void app_main(void)
{
    init_nvs();
    app_state_init();
    ESP_LOGI(TAG, "PC Monitor starting");
    app_serial_cfg_start();
    app_ui_start();
    app_sensor_start();
    app_wifi_start();
    app_time_start();
    app_weather_start();
    app_discovery_start();
    app_pc_monitor_start();
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}
