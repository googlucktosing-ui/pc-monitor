import os

# 1. Update pc_server.py to read from VERSION file
with open("pc_server/pc_server.py", "r", encoding="utf-8") as f:
    c = f.read()

old = "__version__ = \"1.0.0\""
new = """import os as _os
_version_file = _os.path.join(_os.path.dirname(_os.path.abspath(__file__)), "..", "VERSION")
try:
    with open(_version_file) as _f:
        __version__ = _f.read().strip()
except Exception:
    __version__ = "1.0.0"
del _os, _version_file, _f"""

if old in c:
    c = c.replace(old, new)
    with open("pc_server/pc_server.py", "w", encoding="utf-8") as f:
        f.write(c)
    print("pc_server.py updated!")
else:
    print("Pattern not found!")
    idx = c.find("__version__")
    if idx >= 0:
        print(repr(c[idx:idx+40]))

# 2. Update pc_server_tray.py import to also import from VERSION
with open("pc_server/pc_server_tray.py", "r", encoding="utf-8") as f:
    c2 = f.read()

# The import of __version__ already works via `from pc_server import __version__`
# But TrayApp also uses it directly at line 897
# Let's add a version property to TrayApp
old2 = "        log.info(f\"TrayApp init: ws://{host}:{port} | PC IP={self._local_ip} | \""
new2 = "        log.info(f\"PC Monitor Tray v{__version__}: ws://{host}:{port} | PC IP={self._local_ip} | \""
if old2 in c2:
    c2 = c2.replace(old2, new2)
    with open("pc_server/pc_server_tray.py", "w", encoding="utf-8") as f:
        f.write(c2)
    print("pc_server_tray.py log line updated!")

# 3. Add VERSION.h for ESP32
version = open("VERSION", encoding="utf-8").read().strip()
vh = f"""// Auto-generated from VERSION file - DO NOT EDIT MANUALLY
#ifndef VERSION_H
#define VERSION_H

#define APP_VERSION "{version}"

#endif
"""
with open("pc_monitor/main/version.h", "w", encoding="utf-8") as f:
    f.write(vh)
print(f"version.h created with v{version}")

# 4. Add version include to main.c
with open("pc_monitor/main/main.c", "r", encoding="utf-8") as f:
    mc = f.read()
if '#include "version.h"' not in mc:
    mc = mc.replace('#include "app_config.h"', '#include "app_config.h"\n#include "version.h"')
    mc = mc.replace("int app_main(void)", "    ESP_LOGI(\"PC_MONITOR\", \"Firmware v\" APP_VERSION);\nint app_main(void)")
    with open("pc_monitor/main/main.c", "w", encoding="utf-8") as f:
        f.write(mc)
    print("main.c updated!")

print("Done!")
