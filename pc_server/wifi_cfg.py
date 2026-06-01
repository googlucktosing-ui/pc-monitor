cfg = '''#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define APP_WIFI_SSID              "TP-LINK_AX3000"
#define APP_WIFI_PASSWORD          "sing15013387640"
#define APP_PC_SERVER_URI          "ws://192.168.1.105:9090"
#define APP_DHT11_GPIO             25
#define APP_SENSOR_PERIOD_MS       2000
#define APP_TIME_SYNC_PERIOD_MS    (6 * 60 * 60 * 1000)
#define APP_TIMEZONE               "CST-8"
#define APP_UI_REFRESH_MS          500

#endif
'''
with open('/home/esp32/esp32/esp32-board/pc_monitor/main/app_config.h', 'w') as f:
    f.write(cfg.lstrip())
print('Updated')