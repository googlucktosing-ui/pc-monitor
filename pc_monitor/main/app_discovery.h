#ifndef APP_DISCOVERY_H
#define APP_DISCOVERY_H

#include <stdbool.h>

void app_discovery_start(void);
const char *app_discovery_get_uri(void);
bool app_discovery_found(void);
void app_discovery_reset(void);
void app_discovery_report_failure(void);

#endif
