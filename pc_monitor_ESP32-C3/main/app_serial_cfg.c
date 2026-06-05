#include "app_serial_cfg.h"
#include "app_config.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

static const char *TAG = "serial_cfg";
#define SERIAL_BUF_SIZE 256
#define NVS_NAMESPACE    "wifi_cfg"

static void save_wifi_to_nvs(const char *ssid, const char *password)
{
    nvs_handle_t nvs;
    esp_err_t err;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_set_str(nvs, "ssid", ssid);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS set ssid failed: %s", esp_err_to_name(err));
    }

    err = nvs_set_str(nvs, "password", password);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS set password failed: %s", esp_err_to_name(err));
    }

    err = nvs_commit(nvs);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(err));
    }

    nvs_close(nvs);
    ESP_LOGI(TAG, "WiFi saved to NVS: ssid=%s", ssid);
}

void app_wifi_load_from_nvs(char *ssid_buf, size_t ssid_len,
                             char *pass_buf, size_t pass_len)
{
    nvs_handle_t nvs;
    esp_err_t err;

    snprintf(ssid_buf, ssid_len, "%s", APP_WIFI_SSID);
    snprintf(pass_buf, pass_len, "%s", APP_WIFI_PASSWORD);

    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "No saved WiFi config, using defaults");
        return;
    }

    size_t len = ssid_len;
    err = nvs_get_str(nvs, "ssid", ssid_buf, &len);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No ssid in NVS, using default");
        nvs_close(nvs);
        return;
    }

    len = pass_len;
    err = nvs_get_str(nvs, "password", pass_buf, &len);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No password in NVS, using default");
        nvs_close(nvs);
        return;
    }

    nvs_close(nvs);
    ESP_LOGI(TAG, "Loaded WiFi from NVS: ssid=%s", ssid_buf);
}

static void serial_cfg_task(void *arg)
{
    uint8_t buf[SERIAL_BUF_SIZE];
    size_t pos = 0;

    /* 重要: 不要直接调用uart_read_bytes或usb_serial_jtag_read_bytes
       控制台底层已安装驱动，通过stdin读取即可 */
    setvbuf(stdin, NULL, _IONBF, 0);

    ESP_LOGI(TAG, "Serial config listener started (stdin mode)");
    ESP_LOGI(TAG, "Send via serial: WIFI:SSID:PASSWORD to configure WiFi");

    while (1) {
        int ch = getchar();
        if (ch == EOF) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        uint8_t c = (uint8_t)ch;

        if (c == '\n' || c == '\r') {
            if (pos > 0) {
                buf[pos] = 0;
                ESP_LOGI(TAG, "Received (%d bytes): %s", pos, (const char *)buf);

                if (strncmp((const char *)buf, "WIFI:", 5) == 0) {
                    char *ssid = (char *)buf + 5;
                    char *password = strchr(ssid, ':');
                    if (password) {
                        *password++ = 0;
                        if (strlen(ssid) > 0 && strlen(password) > 0) {
                            save_wifi_to_nvs(ssid, password);
                            printf("OK\n");
                            fflush(stdout);
                            ESP_LOGI(TAG, "WiFi saved! Rebooting...");
                            vTaskDelay(pdMS_TO_TICKS(200));
                            esp_restart();
                        } else {
                            printf("ERROR: empty\n");
                            fflush(stdout);
                        }
                    } else {
                        printf("ERROR: need WIFI:S:P\n");
                        fflush(stdout);
                    }
                } else {
                    printf("ERROR: unknown\n");
                    fflush(stdout);
                }
                pos = 0;
            }
        } else if (pos < SERIAL_BUF_SIZE - 1) {
            buf[pos++] = c;
        }
    }
}

void app_serial_cfg_start(void)
{
    xTaskCreate(serial_cfg_task, "serial_cfg", 4096, NULL, 1, NULL);
}
