#include "app_weather.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "app_config.h"
#include "app_state.h"
#include "app_wifi.h"

#define WEATHER_BUFFER_SIZE 2048

static const char *TAG = "weather";

typedef struct {
    char data[WEATHER_BUFFER_SIZE];
    int len;
} weather_http_buffer_t;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    weather_http_buffer_t *buf = (weather_http_buffer_t *)evt->user_data;

    if (evt->event_id == HTTP_EVENT_ON_DATA && buf && evt->data_len > 0) {
        int copy_len = evt->data_len;
        if (buf->len + copy_len >= WEATHER_BUFFER_SIZE) {
            copy_len = WEATHER_BUFFER_SIZE - buf->len - 1;
        }
        if (copy_len > 0) {
            memcpy(buf->data + buf->len, evt->data, copy_len);
            buf->len += copy_len;
            buf->data[buf->len] = '\0';
        }
    }
    return ESP_OK;
}

static bool parse_weather(const char *json)
{
    bool ok = false;
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        ESP_LOGW(TAG, "Invalid weather JSON");
        return false;
    }

    cJSON *results = cJSON_GetObjectItem(root, "results");
    cJSON *first = cJSON_IsArray(results) ? cJSON_GetArrayItem(results, 0) : NULL;
    cJSON *location = first ? cJSON_GetObjectItem(first, "location") : NULL;
    cJSON *now = first ? cJSON_GetObjectItem(first, "now") : NULL;
    cJSON *city = location ? cJSON_GetObjectItem(location, "name") : NULL;
    cJSON *text = now ? cJSON_GetObjectItem(now, "text") : NULL;
    cJSON *temp = now ? cJSON_GetObjectItem(now, "temperature") : NULL;

    if (cJSON_IsString(city) && cJSON_IsString(text) && cJSON_IsString(temp)) {
        app_state_set_weather(city->valuestring, text->valuestring, atoi(temp->valuestring), true);
        ESP_LOGI(TAG, "weather %s %s %sC", city->valuestring, text->valuestring, temp->valuestring);
        ok = true;
    } else {
        ESP_LOGW(TAG, "Weather JSON missing fields");
    }

    cJSON_Delete(root);
    return ok;
}

static bool fetch_weather_once(void)
{
    if (strlen(APP_WEATHER_API_KEY) == 0) {
        app_state_set_weather(APP_WEATHER_CITY, "No API key", 0, false);
        ESP_LOGW(TAG, "Set APP_WEATHER_API_KEY in app_config.h");
        return false;
    }

    if (!app_wifi_wait_connected(15000)) {
        ESP_LOGW(TAG, "No WiFi for weather");
        return false;
    }

    char url[256];
    snprintf(
        url,
        sizeof(url),
        "https://api.seniverse.com/v3/weather/now.json?key=%s&location=%s&language=en&unit=c",
        APP_WEATHER_API_KEY,
        APP_WEATHER_CITY
    );

    weather_http_buffer_t buffer = { 0 };
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .user_data = &buffer,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        return false;
    }

    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (err != ESP_OK || status < 200 || status >= 300) {
        ESP_LOGW(TAG, "Weather request failed err=%s status=%d", esp_err_to_name(err), status);
        return false;
    }

    return parse_weather(buffer.data);
}

static void weather_task(void *arg)
{
    (void)arg;
    app_state_set_weather(APP_WEATHER_CITY, "--", 0, false);

    while (1) {
        bool ok = fetch_weather_once();
        if (!ok) {
        }
        vTaskDelay(pdMS_TO_TICKS(APP_WEATHER_PERIOD_MS));
    }
}

void app_weather_start(void)
{
    xTaskCreatePinnedToCore(weather_task, "weather_task", 6144, NULL, 3, NULL, 0);
}

