import re

BASE = '/home/esp32/esp32/esp32-board/pc_monitor/main'

with open(BASE + '/app_ui.c', 'r') as f:
    content = f.read()

# 1. Add s_wc_indoor_title static variable
content = content.replace(
    'static lv_obj_t *s_wc_weather;',
    'static lv_obj_t *s_wc_indoor_title;\nstatic lv_obj_t *s_wc_weather;'
)

# 2. Save indoor_title reference
content = content.replace(
    '    lv_obj_t *indoor_title = lv_label_create(parent);',
    '    s_wc_indoor_title = lv_label_create(parent);'
)

# 3. Fix refresh_weather_clock - hide INDOOR when no sensor
old_rwc = '''    /* Indoor temp */
    if (st.sensor_valid) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), " %d.%d C\, st.indoor_temp_x10 / 10, st.indoor_temp_x10 % 10);
 lv_label_set_text(s_wc_indoor_temp, tmp);
 snprintf(tmp, sizeof(tmp), \Humidity %d%%\, st.indoor_hum);
 lv_label_set_text(s_wc_hum, tmp);
 } else {
 lv_label_set_text(s_wc_indoor_temp, \--.- C\);
 lv_label_set_text(s_wc_hum, \Humidity --%\);
 }'''

new_rwc = ''' /* Indoor temp */
 if (st.sensor_valid) {
 char tmp[16];
 snprintf(tmp, sizeof(tmp), \%d.%d C\, st.indoor_temp_x10 / 10, st.indoor_temp_x10 % 10);
 lv_label_set_text(s_wc_indoor_temp, tmp);
 snprintf(tmp, sizeof(tmp), \Humidity %d%%\, st.indoor_hum);
 lv_label_set_text(s_wc_hum, tmp);
 lv_obj_clear_flag(s_wc_indoor_title, LV_OBJ_FLAG_HIDDEN);
 lv_obj_clear_flag(s_wc_indoor_temp, LV_OBJ_FLAG_HIDDEN);
 lv_obj_clear_flag(s_wc_hum, LV_OBJ_FLAG_HIDDEN);
 } else {
 lv_obj_add_flag(s_wc_indoor_title, LV_OBJ_FLAG_HIDDEN);
 lv_obj_add_flag(s_wc_indoor_temp, LV_OBJ_FLAG_HIDDEN);
 lv_obj_add_flag(s_wc_hum, LV_OBJ_FLAG_HIDDEN);
 }'''
content = content.replace(old_rwc, new_rwc)

# 4. Fix GPU section - hide when no GPU
old_gpu = ''' /* GPU -- show CPU temp when no GPU detected */
 if (st.pc_connected && st.gpu_name[0] == 0) {
 /* No GPU detected - show CPU mode */
 snprintf(buf, sizeof(buf), \%.0f%%\, st.cpu_usage);
 lv_label_set_text(s_pc_gpu_val, buf);
 if (st.cpu_temp > 0) {
 snprintf(buf, sizeof(buf), \%.0f C\, st.cpu_temp);
 } else {
 snprintf(buf, sizeof(buf), \-- C\);
 }
 lv_label_set_text(s_pc_gpu_temp_label, buf);
 lv_label_set_text(s_pc_gpu_title, \CPU\);
 lv_obj_set_style_text_color(s_pc_gpu_title, lv_color_hex(0x60A5FA), 0);
 } else {
 snprintf(buf, sizeof(buf), \%.0f%%\, st.gpu_usage);
 lv_label_set_text(s_pc_gpu_val, buf);
 if (st.pc_connected && st.gpu_temp > 0) {
 snprintf(buf, sizeof(buf), \%.0f C\, st.gpu_temp);
 } else {
 snprintf(buf, sizeof(buf), \-- C\);
 }
 lv_label_set_text(s_pc_gpu_temp_label, buf);
 lv_label_set_text(s_pc_gpu_title, \GPU\);
 lv_obj_set_style_text_color(s_pc_gpu_title, lv_color_hex(0x8B5CF6), 0);
 }
 lv_bar_set_value(s_pc_gpu_bar, (int)((st.gpu_name[0] == 0 ? st.cpu_usage : st.gpu_usage) + 0.5f), LV_ANIM_OFF);'''

new_gpu = ''' /* GPU -- hide section when no GPU detected */
 if (st.pc_connected && st.gpu_name[0] != 0 && st.gpu_name[0] != '-') {
 /* GPU present */
 lv_obj_clear_flag(s_pc_gpu_title, LV_OBJ_FLAG_HIDDEN);
 lv_obj_clear_flag(s_pc_gpu_val, LV_OBJ_FLAG_HIDDEN);
 lv_obj_clear_flag(s_pc_gpu_bar, LV_OBJ_FLAG_HIDDEN);
 lv_obj_clear_flag(s_pc_gpu_temp_label, LV_OBJ_FLAG_HIDDEN);
 snprintf(buf, sizeof(buf), \%.0f%%\, st.gpu_usage);
 lv_label_set_text(s_pc_gpu_val, buf);
 if (st.pc_connected && st.gpu_temp > 0) {
 snprintf(buf, sizeof(buf), \%.0f C\, st.gpu_temp);
 } else {
 snprintf(buf, sizeof(buf), \-- C\);
 }
 lv_label_set_text(s_pc_gpu_temp_label, buf);
 lv_label_set_text(s_pc_gpu_title, \GPU\);
 lv_obj_set_style_text_color(s_pc_gpu_title, lv_color_hex(0x8B5CF6), 0);
 lv_bar_set_value(s_pc_gpu_bar, (int)(st.gpu_usage + 0.5f), LV_ANIM_OFF);
 } else if (st.pc_connected && st.cpu_temp > 0) {
 /* No GPU but CPU temp available - show CPU temp instead */
 lv_obj_clear_flag(s_pc_gpu_title, LV_OBJ_FLAG_HIDDEN);
 lv_obj_clear_flag(s_pc_gpu_val, LV_OBJ_FLAG_HIDDEN);
 lv_obj_clear_flag(s_pc_gpu_bar, LV_OBJ_FLAG_HIDDEN);
 lv_obj_clear_flag(s_pc_gpu_temp_label, LV_OBJ_FLAG_HIDDEN);
 snprintf(buf, sizeof(buf), \%.0f\\u00b0C\, st.cpu_temp);
 lv_label_set_text(s_pc_gpu_val, buf);
 lv_label_set_text(s_pc_gpu_temp_label, \CPU Temp\);
 lv_label_set_text(s_pc_gpu_title, \CPU Temp\);
 lv_obj_set_style_text_color(s_pc_gpu_title, lv_color_hex(0x60A5FA), 0);
 lv_bar_set_value(s_pc_gpu_bar, (int)(st.cpu_temp + 0.5f), LV_ANIM_OFF);
 } else {
 /* No GPU and no CPU temp - hide entire GPU section */
 lv_obj_add_flag(s_pc_gpu_title, LV_OBJ_FLAG_HIDDEN);
 lv_obj_add_flag(s_pc_gpu_val, LV_OBJ_FLAG_HIDDEN);
 lv_obj_add_flag(s_pc_gpu_bar, LV_OBJ_FLAG_HIDDEN);
 lv_obj_add_flag(s_pc_gpu_temp_label, LV_OBJ_FLAG_HIDDEN);
 }'''
content = content.replace(old_gpu, new_gpu)

with open(BASE + '/app_ui.c', 'w') as f:
 f.write(content)

print('Done')
