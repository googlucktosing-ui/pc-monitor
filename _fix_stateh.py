import re, os

with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_state.h", encoding="utf-8") as f:
    c = f.read()
c = c.replace("    int outdoor_temp;", "    int outdoor_temp;\n    int theme;")
c = c.replace("void app_state_set_weather(const char *city, const char *weather, int temp, bool valid);",
              "void app_state_set_weather(const char *city, const char *weather, int temp, bool valid);\nvoid app_state_set_theme(int theme);")
with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_state.h", "w", encoding="utf-8") as f:
    f.write(c)
print("app_state.h OK", os.path.getsize("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_state.h"), "bytes")