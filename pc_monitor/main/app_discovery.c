#include "app_discovery.h"
#include "app_config.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"
#include <string.h>
#include "freertos/task.h"
#include <stdio.h>

static const char *TAG = "discovery";
static char s_discovered_uri[64] = {0};
static volatile bool s_discovered = false;
static volatile int s_fail_count = 0;

#define MAX_FAILURES_BEFORE_CLEAR  2
#define DISCOVERY_QUERY_INTERVAL   3000
#define DISCOVERY_LISTEN_TIMEOUT   1500

/* ---------- NVS Cache ---------- */
static void cache_save(const char *ip, int port)
{
    nvs_handle_t h;
    if (nvs_open("pc_monitor", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_str(h, APP_NVS_CACHE_KEY, ip);
        nvs_set_i32(h, APP_NVS_CACHE_PORT_KEY, port);
        nvs_commit(h);
        nvs_close(h);
    }
}

static bool cache_load(char *ip_buf, size_t ip_len, int *port)
{
    nvs_handle_t h;
    if (nvs_open("pc_monitor", NVS_READONLY, &h) != ESP_OK)
        return false;
    size_t sz = ip_len;
    bool ok = (nvs_get_str(h, APP_NVS_CACHE_KEY, ip_buf, &sz) == ESP_OK &&
               nvs_get_i32(h, APP_NVS_CACHE_PORT_KEY, port) == ESP_OK);
    nvs_close(h);
    return ok;
}

static void cache_clear(void)
{
    nvs_handle_t h;
    if (nvs_open("pc_monitor", NVS_READWRITE, &h) == ESP_OK) {
        nvs_erase_key(h, APP_NVS_CACHE_KEY);
        nvs_erase_key(h, APP_NVS_CACHE_PORT_KEY);
        nvs_commit(h);
        nvs_close(h);
    }
    ESP_LOGI(TAG, "NVS cache cleared");
}

/* ---------- Parse IP|PORT from string, discover PC ---------- */
static bool parse_discover(const char *str)
{
    if (!str || !*str) return false;
    const char *pipe = strchr(str, '|');
    if (!pipe || pipe == str) return false;
    size_t ip_len = pipe - str;
    if (ip_len < 7 || ip_len > 31) return false;
    const char *dot = (const char *)memchr(str, '.', ip_len);
    if (!dot) return false;

    char ip_buf[32];
    memcpy(ip_buf, str, ip_len);
    ip_buf[ip_len] = '\0';
    int port = atoi(pipe + 1);
    if (port <= 0) port = APP_PC_SERVER_PORT;

    snprintf(s_discovered_uri, sizeof(s_discovered_uri), "ws://%s:%d", ip_buf, port);
    s_discovered = true;
    s_fail_count = 0;
    ESP_LOGI(TAG, "Discovered PC: %s", s_discovered_uri);
    cache_save(ip_buf, port);
    return true;
}

/* ---------- Try NVS cached PC IP (instant reconnect) ---------- */
static bool try_cache(void)
{
    char ip[32];
    int port = 0;
    if (!cache_load(ip, sizeof(ip), &port) || port <= 0)
        return false;
    snprintf(s_discovered_uri, sizeof(s_discovered_uri), "ws://%s:%d", ip, port);
    s_discovered = true;
    ESP_LOGI(TAG, "NVS cache connect: %s:%d", ip, port);
    return true;
}

/* perpetual discovery task - runs forever */
static void discovery_task(void *arg)
{
    (void)arg;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Socket creation failed");
        vTaskDelete(NULL);
        return;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    struct timeval tv = {
        .tv_sec  = DISCOVERY_LISTEN_TIMEOUT / 1000,
        .tv_usec = (DISCOVERY_LISTEN_TIMEOUT % 1000) * 1000
    };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in bind_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(APP_DISCOVERY_PORT),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    if (bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
        ESP_LOGE(TAG, "Bind failed on port %d", APP_DISCOVERY_PORT);
        close(sock);
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in bcast = {
        .sin_family = AF_INET,
        .sin_port = htons(APP_DISCOVERY_PORT),
        .sin_addr.s_addr = htonl(INADDR_BROADCAST),
    };

    ESP_LOGI(TAG, "Perpetual discovery on UDP %d", APP_DISCOVERY_PORT);

    char buf[256];
    struct sockaddr_in from;
    socklen_t from_len;
    TickType_t last_query = 0;

    /* NEVER EXITS */
    while (1) {
        from_len = sizeof(from);
        int len = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                           (struct sockaddr *)&from, &from_len);

        if (len < 0) {
            /* Timeout - send periodic query */
            TickType_t now = xTaskGetTickCount();
            if (now - last_query >= pdMS_TO_TICKS(DISCOVERY_QUERY_INTERVAL)) {
                last_query = now;
                sendto(sock, APP_DISCOVERY_QUERY, strlen(APP_DISCOVERY_QUERY), 0,
                       (struct sockaddr *)&bcast, sizeof(bcast));
            }
            continue;
        }

        buf[len] = '\0';

        size_t mlen = strlen(APP_DISCOVERY_MAGIC);

        /* Case A: PC broadcast "PC_MONITOR|IP|PORT" */
        if (strncmp(buf, APP_DISCOVERY_MAGIC, mlen) == 0) {
            if (buf[mlen] == '|') {
                parse_discover(buf + mlen + 1);
            }
            continue;
        }

        /* Case B: PC query response "PC_MONITOR_HERE|IP|PORT" */
        size_t rlen = strlen(APP_DISCOVERY_RESP_PREFIX);
        if (strncmp(buf, APP_DISCOVERY_RESP_PREFIX, rlen) == 0) {
            parse_discover(buf + rlen);
            continue;
        }
    }

    /* unreachable */
    close(sock);
}

/* -- Public API -- */

void app_discovery_start(void)
{
    ESP_LOGI(TAG, "Starting perpetual discovery...");
    try_cache();
    xTaskCreate(discovery_task, "disc", 4096, NULL, 4, NULL);
}

const char *app_discovery_get_uri(void)
{
    return s_discovered ? s_discovered_uri : NULL;
}

bool app_discovery_found(void)
{
    return s_discovered;
}

void app_discovery_reset(void)
{
    s_discovered = false;
    memset(s_discovered_uri, 0, sizeof(s_discovered_uri));
    ESP_LOGI(TAG, "Discovery reset");
}

void app_discovery_report_failure(void)
{
    s_fail_count++;
    ESP_LOGW(TAG, "Conn fail %d/%d", s_fail_count, MAX_FAILURES_BEFORE_CLEAR);
    if (s_fail_count >= MAX_FAILURES_BEFORE_CLEAR) {
        cache_clear();
        app_discovery_reset();
        s_fail_count = 0;
        ESP_LOGI(TAG, "Cache cleared, perpetual discovery active");
    }
}
