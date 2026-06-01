import os, sys
sys.stdout.reconfigure(encoding="utf-8")

# Read version cleanly
with open("VERSION", "r", encoding="utf-8-sig") as f:
    ver = f.read().strip()
print(f"Version: {ver}")

# Create version.h for ESP32
vh = "// Auto-generated from VERSION file\n"
vh += "#ifndef VERSION_H\n#define VERSION_H\n"
vh += f'#define APP_VERSION "{ver}"\n'
vh += "#endif\n"
with open("pc_monitor/main/version.h", "w", encoding="utf-8") as f:
    f.write(vh)
print("version.h created")

# Update main.c - add include and log
with open("pc_monitor/main/main.c", "r", encoding="utf-8") as f:
    mc = f.read()

to_add = '#include "version.h"\n'
if to_add.strip() not in mc:
    mc = mc.replace('#include "app_config.h"', '#include "app_config.h"\n' + to_add)
    mc = mc.replace("int app_main(void)", '    ESP_LOGI("PC_MONITOR", "Firmware v" APP_VERSION);\nint app_main(void)')
    with open("pc_monitor/main/main.c", "w", encoding="utf-8") as f:
        f.write(mc)
    print("main.c updated!")
else:
    print("main.c already has version.h include")

print("All done!")
