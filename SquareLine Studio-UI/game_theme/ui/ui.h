#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/* Screen */
extern lv_obj_t *ui_Screen1;

/* Top bar */
extern lv_obj_t *ui_TopBar;
extern lv_obj_t *ui_cpu_temp_title;
extern lv_obj_t *ui_cpu_temp_val;
extern lv_obj_t *ui_gpu_temp_title;
extern lv_obj_t *ui_gpu_temp_val;
extern lv_obj_t *ui_perf_title;
extern lv_obj_t *ui_perf_val;

/* CPU gauge */
extern lv_obj_t *ui_cpu_card;
extern lv_obj_t *ui_cpu_arc;
extern lv_obj_t *ui_cpu_pct;
extern lv_obj_t *ui_cpu_label;
extern lv_obj_t *ui_cpu_freq;

/* GPU gauge */
extern lv_obj_t *ui_gpu_card;
extern lv_obj_t *ui_gpu_arc;
extern lv_obj_t *ui_gpu_pct;
extern lv_obj_t *ui_gpu_label;
extern lv_obj_t *ui_gpu_freq;

/* Info panel */
extern lv_obj_t *ui_info_panel;
extern lv_obj_t *ui_ram_label;
extern lv_obj_t *ui_ram_bar;
extern lv_obj_t *ui_net_down;
extern lv_obj_t *ui_net_up;

/* Status bar */
extern lv_obj_t *ui_status_bar;
extern lv_obj_t *ui_status_left;
extern lv_obj_t *ui_status_mid;
extern lv_obj_t *ui_status_right;

/* Function declarations */
void ui_Screen1_screen_init(void);
void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*UI_H*/
