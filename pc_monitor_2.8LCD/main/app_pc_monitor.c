#include "app_pc_monitor.h"
#include "app_state.h"
#include "app_config.h"
#include "app_wifi.h"
#include "app_discovery.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_websocket_client.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "pc_monitor";
static esp_websocket_client_handle_t s_client = NULL;

static void handle_pc_data(const char *data, int len)
{
    cJSON *root = cJSON_ParseWithLength(data, len);
    if (!root) return;

    float cpu=0,gpu=0,gpu_temp=0,cpu_temp=0,mu=0,mt=0,du=0,dt=0,nu=0,nd=0;
    char hn[32]="PC",cn[48]="---",gn[48]="---",oi[48]="---";
    cJSON *i;
    #define GETNUM(f,k) do{ i=cJSON_GetObjectItem(root,k); if(cJSON_IsNumber(i)) f=(float)i->valuedouble; }while(0)
    #define GETSTR(s,k) do{ i=cJSON_GetObjectItem(root,k); if(cJSON_IsString(i)) strncpy(s,i->valuestring,sizeof(s)-1); }while(0)
    GETNUM(cpu,"cpu"); GETNUM(gpu,"gpu"); GETNUM(gpu_temp,"gpu_temp"); GETNUM(cpu_temp,"cpu_temp");
    GETNUM(mu,"mem_used"); GETNUM(mt,"mem_total"); GETNUM(du,"disk_used"); GETNUM(dt,"disk_total");
    GETNUM(nu,"net_up"); GETNUM(nd,"net_down");
    GETSTR(hn,"hostname"); GETSTR(cn,"cpu_name"); GETSTR(gn,"gpu_name"); GETSTR(oi,"os_info");
    ESP_LOGI(TAG, "PC data: cpu=%.0f%% cpu_temp=%.0fC mem=%.1f/%.1fGB host=%s", cpu, cpu_temp, mu, mt, hn);
    app_state_set_pc_data(cpu,gpu,gpu_temp,cpu_temp,mu,mt,du,dt,nu,nd,hn,cn,gn,oi,true);
    cJSON_Delete(root);
}

static void ws_handler(void *h, esp_event_base_t b, int32_t eid, void *ed)
{
    (void)h;(void)b;
    esp_websocket_event_data_t *d = (esp_websocket_event_data_t *)ed;
    switch (eid) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Connected to PC");
        app_state_set_pc_data(0,0,0,0,0,0,0,0,0,0,NULL,NULL,NULL,NULL,true);
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected from PC");
        app_state_set_pc_data(0,0,0,0,0,0,0,0,0,0,NULL,NULL,NULL,NULL,false);
        break;
    case WEBSOCKET_EVENT_DATA:
        if (d->op_code==1 && d->data_len>0) {
            char *buf = malloc(d->data_len+1);
            if (buf) { memcpy(buf,d->data_ptr,d->data_len); buf[d->data_len]=0; handle_pc_data(buf,d->data_len); free(buf); }
        }
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGE(TAG, "WebSocket error");
        break;
    default: break;
    }
}

static void task(void *arg)
{
    (void)arg;

    while (1) {
        /* Wait for WiFi */
        while (!app_wifi_wait_connected(30000)) { vTaskDelay(pdMS_TO_TICKS(3000)); }
        ESP_LOGI(TAG, "WiFi OK");

        /* Get URI from discovery (includes NVS cache + perpetual discovery) */
        const char *uri = app_discovery_get_uri();
        if (!uri) {
            /* Wait for perpetual discovery to find PC (up to 15s) */
            int n = 30;  /* 30 * 500ms = 15s */
            ESP_LOGI(TAG, "Waiting for discovery...");
            while (!app_discovery_found() && --n > 0) vTaskDelay(pdMS_TO_TICKS(500));
            uri = app_discovery_get_uri();
        }
        if (!uri) {
            ESP_LOGW(TAG, "Discovery timeout, will retry in 3s");
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
        }
        ESP_LOGI(TAG, "Connect: %s", uri);

        esp_websocket_client_config_t cfg = {
            .task_stack = 4096,
            .buffer_size = 2048,
            .reconnect_timeout_ms = 3000,
        };
        cfg.uri = uri;
        s_client = esp_websocket_client_init(&cfg);
        esp_websocket_register_events(s_client, WEBSOCKET_EVENT_ANY, ws_handler, NULL);
        esp_websocket_client_start(s_client);

        /* Wait up to 10s for connection */
        int ok_wait = 0;
        while (ok_wait < 20) {  /* 20 * 500ms = 10s */
            vTaskDelay(pdMS_TO_TICKS(500));
            if (esp_websocket_client_is_connected(s_client)) break;
            ok_wait++;
        }

        if (esp_websocket_client_is_connected(s_client)) {
            ESP_LOGI(TAG, "WebSocket connected, monitoring...");
            /* Connected: monitor until disconnect */
            while (1) {
                vTaskDelay(pdMS_TO_TICKS(5000));
                if (!esp_websocket_client_is_connected(s_client)) {
                    ESP_LOGI(TAG, "Lost connection, immediately rediscovering");
                    /* On disconnect: immediately reset discovery so perpetual task finds new IP */
                    app_discovery_reset();
                    break;
                }
            }
        } else {
            ESP_LOGW(TAG, "WebSocket connect failed");
            app_discovery_report_failure();
            vTaskDelay(pdMS_TO_TICKS(1000));  /* Shorter retry delay */
        }

        esp_websocket_client_destroy(s_client);
        s_client = NULL;
    }
}

void app_pc_monitor_start(void)
{
    xTaskCreate(task, "pc_mon", 6144, NULL, 5, NULL);
}
