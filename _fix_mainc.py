with open("pc_monitor/main/main.c", "r", encoding="utf-8") as f:
    mc = f.read()

# Add version.h include after the last #include line
old_include = '#include "app_serial_cfg.h"'
new_include = '#include "app_serial_cfg.h"\n#include "version.h"'
if old_include in mc and '#include "version.h"' not in mc:
    mc = mc.replace(old_include, new_include)
    print("Added version.h include")

# Add version log at start of app_main
old_main = 'void app_main(void)\n{'
new_main = 'void app_main(void)\n{\n    ESP_LOGI("pc_monitor", "Firmware v" APP_VERSION);'
if old_main in mc:
    mc = mc.replace(old_main, new_main)
    print("Added version log in app_main")

with open("pc_monitor/main/main.c", "w", encoding="utf-8") as f:
    f.write(mc)
print("Done!")
