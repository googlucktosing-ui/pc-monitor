#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* ========= WiFi Configuration ========= */
#define APP_WIFI_SSID              "TP-LINK_AX3000"
#define APP_WIFI_PASSWORD          "sing15013387640"

/* ========= PC Server Configuration ========= */
#define APP_PC_SERVER_URI          "ws://192.168.30.140:18090"
#define APP_PC_SERVER_PORT         18090

/* ========= Multi-Layer Discovery ========= */
/* Discovery tries: NVS cache -> UDP query -> UDP listen -> fallback */
#define APP_DISCOVERY_PORT         54789
#define APP_DISCOVERY_MAGIC        "PC_MONITOR"
#define APP_DISCOVERY_QUERY        "PC_MONITOR_QUERY"
#define APP_DISCOVERY_RESP_PREFIX  "PC_MONITOR_HERE|"
#define APP_DISCOVERY_TIMEOUT_MS   30000
#define APP_DISCOVERY_QUERY_MS     3000
#define APP_NVS_CACHE_KEY          "pc_ip_cache"
#define APP_NVS_CACHE_PORT_KEY     "pc_port_cache"

/* ========= Sensor Configuration ========= */
#define APP_DHT11_GPIO             3
#define APP_SENSOR_PERIOD_MS       2000

/* ========= Time Configuration ========= */
#define APP_TIME_SYNC_PERIOD_MS    (6 * 60 * 60 * 1000)
#define APP_TIMEZONE               "CST-8"

/* ========= Weather Configuration (Seniverse API) ========= */
#define APP_WEATHER_API_KEY        "75b553591cd14b02a7978d6048396c0f"
#define APP_WEATHER_CITY           "shenzhen"
#define APP_WEATHER_PERIOD_MS      (20 * 60 * 1000)

/* ========= UI Configuration ========= */
#define APP_UI_REFRESH_MS          500

#endif
