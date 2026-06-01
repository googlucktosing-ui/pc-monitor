#ifndef APP_SERIAL_CFG_H
#define APP_SERIAL_CFG_H

void app_serial_cfg_start(void);
void app_wifi_load_from_nvs(char *ssid_buf, size_t ssid_len,
                             char *pass_buf, size_t pass_len);

#endif
