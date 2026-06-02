#ifndef APP_WIFI_H
#define APP_WIFI_H

#include <stdbool.h>

void app_wifi_start(void);
bool app_wifi_wait_connected(int timeout_ms);

#endif
