import re
path = '/home/esp32/esp32/esp32-board/pc_monitor/components/lvgl/lv_conf.h'
with open(path, 'r') as f:
    content = f.read()
content = content.replace('#if 0 /*Set it to "1" to enable content*/', '#if 1 /*Set it to "1" to enable content*/')
content = re.sub(r'#define LV_COLOR_DEPTH \d+', '#define LV_COLOR_DEPTH 16', content)
content = re.sub(r'#define LV_USE_DISPLAY \d+', '#define LV_USE_DISPLAY  1', content)
for sz in [12, 14, 16, 18, 20, 22, 28]:
    content = re.sub(r'#define LV_FONT_MONTSERRAT_' + str(sz) + r' \d+', '#define LV_FONT_MONTSERRAT_' + str(sz) + '  1', content)
content = re.sub(r'#define LV_USE_LOG \d+', '#define LV_USE_LOG  1', content)
content = re.sub(r'#define LV_USE_DEMO_BENCHMARK \d+', '#define LV_USE_DEMO_BENCHMARK 0', content)
content = re.sub(r'#define LV_USE_DEMO_MUSIC \d+', '#define LV_USE_DEMO_MUSIC 0', content)
content = re.sub(r'#define LV_USE_DEMO_WIDGETS \d+', '#define LV_USE_DEMO_WIDGETS 0', content)
with open(path, 'w') as f:
    f.write(content)
print('lv_conf.h configured')