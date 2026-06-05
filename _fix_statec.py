import re, os

with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_state.c", encoding="utf-8") as f:
    c = f.read()

weather_start = c.find("void app_state_set_weather")
weather_end = c.find("\n}", weather_start) + 3
weather_func = c[weather_start:weather_end]

theme_func = """
void app_state_set_theme(int theme)
{
    if (theme < 0) theme = 0;
    if (theme > 2) theme = 2;
    s_state.theme = theme;
}"""

c = c[:weather_end] + "\n" + theme_func + c[weather_end:]

with open("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_state.c", "w", encoding="utf-8") as f:
    f.write(c)
print("app_state.c OK", os.path.getsize("E:/CODEX-Pj/ESP32/pc_monitor_ESP32-C3/main/app_state.c"), "bytes")