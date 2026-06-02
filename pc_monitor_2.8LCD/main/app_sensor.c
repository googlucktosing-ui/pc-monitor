#include "app_sensor.h"

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "app_config.h"
#include "app_state.h"
#include "dht11.h"

static const char *TAG = "sensor";

/* DHT11 occasionally glitches. Use a high threshold and hold valid data
   for a minimum number of cycles to prevent UI flicker. */
#define SENSOR_MAX_FAILURES   15   /* ~30s of failures before marking invalid */
#define SENSOR_MIN_HOLD       5    /* keep showing valid data for ~10s after last good read */

static void sensor_task(void *arg)
{
    (void)arg;
    int temp = 0;
    int hum = 0;
    int fail_count = 0;
    int hold_count = 0;          /* cycles since last valid data while holding */
    bool last_valid = false;
    int last_temp = 0, last_hum = 0;

    DHT11_Init(APP_DHT11_GPIO);
    ESP_LOGI(TAG, "DHT11 started on GPIO %d", APP_DHT11_GPIO);

    while (1) {
        bool ok = DHT11_StartGet(&temp, &hum);
        if (ok) {
            fail_count = 0;
            hold_count = 0;
            last_temp = temp;
            last_hum = hum;
            last_valid = true;
            app_state_set_sensor(temp, hum, true);
            ESP_LOGI(TAG, "indoor temp=%d.%dC hum=%d%%", temp / 10, temp % 10, hum);
        } else {
            fail_count++;
            if (fail_count >= SENSOR_MAX_FAILURES) {
                /* Truly lost - show invalid */
                app_state_set_sensor(0, 0, false);
                last_valid = false;
                ESP_LOGW(TAG, "DHT11 sensor lost after %d failures", fail_count);
            } else if (last_valid && hold_count < SENSOR_MIN_HOLD) {
                /* Keep showing last known good value for SENSOR_MIN_HOLD cycles */
                hold_count++;
                app_state_set_sensor(last_temp, last_hum, true);
                ESP_LOGW(TAG, "DHT11 glitch %d/%d (holding last value)", fail_count, SENSOR_MAX_FAILURES);
            } else {
                /* Beyond hold window, still valid data but we have nothing fresh */
                app_state_set_sensor(0, 0, false);
                ESP_LOGW(TAG, "DHT11 read failed (no hold)");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(APP_SENSOR_PERIOD_MS));
    }
}

void app_sensor_start(void)
{
    xTaskCreatePinnedToCore(sensor_task, "sensor_task", 4096, NULL, 4, NULL, 1);
}
