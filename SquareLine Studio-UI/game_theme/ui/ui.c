#include "ui.h"

/* Screen object */
lv_obj_t *ui_Screen1;

/* Top bar objects */
lv_obj_t *ui_TopBar;
lv_obj_t *ui_cpu_temp_title;
lv_obj_t *ui_cpu_temp_val;
lv_obj_t *ui_gpu_temp_title;
lv_obj_t *ui_gpu_temp_val;
lv_obj_t *ui_perf_title;
lv_obj_t *ui_perf_val;

/* CPU gauge objects */
lv_obj_t *ui_cpu_card;
lv_obj_t *ui_cpu_arc;
lv_obj_t *ui_cpu_pct;
lv_obj_t *ui_cpu_label;
lv_obj_t *ui_cpu_freq;

/* GPU gauge objects */
lv_obj_t *ui_gpu_card;
lv_obj_t *ui_gpu_arc;
lv_obj_t *ui_gpu_pct;
lv_obj_t *ui_gpu_label;
lv_obj_t *ui_gpu_freq;

/* Info panel objects */
lv_obj_t *ui_info_panel;
lv_obj_t *ui_ram_label;
lv_obj_t *ui_ram_bar;
lv_obj_t *ui_net_down;
lv_obj_t *ui_net_up;

/* Status bar objects */
lv_obj_t *ui_status_bar;
lv_obj_t *ui_status_left;
lv_obj_t *ui_status_mid;
lv_obj_t *ui_status_right;

static lv_obj_t *create_top_bar(lv_obj_t *parent) {
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_pos(cont, 8, 8);
    lv_obj_set_size(cont, 224, 48);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x141B24), LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x00E5FF), LV_PART_MAIN);
    lv_obj_set_style_border_opa(cont, 63, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_set_style_border_side(cont, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
    return cont;
}

static lv_obj_t *create_gauge_card(lv_obj_t *parent, lv_coord_t x, lv_coord_t y) {
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_pos(cont, x, y);
    lv_obj_set_size(cont, 96, 112);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x141B24), LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 12, LV_PART_MAIN);
    lv_obj_set_style_border_width(cont, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    return cont;
}

static lv_obj_t *create_arc_gauge(lv_obj_t *parent, int val, lv_color_t color) {
    lv_obj_t *arc = lv_arc_create(parent);
    lv_obj_set_size(arc, 80, 80);
    lv_obj_align(arc, LV_ALIGN_TOP_MID, 0, 8);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, val);
    lv_arc_set_bg_angles(arc, 135, 405);
    lv_arc_set_rotation(arc, 0);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, 8, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 8, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    return arc;
}

static lv_obj_t *create_center_label(lv_obj_t *parent, const char *text, lv_color_t color, int font_size, lv_coord_t y_off) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, color, 0);
    if (font_size == 28) lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    else if (font_size == 14) lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    else if (font_size == 12) lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    else if (font_size == 10) lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y_off);
    return label;
}

void ui_Screen1_screen_init(void) {
    /* Create Screen1 */
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x0B0E14), LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_Screen1, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);

    /* === Top status bar === */
    ui_TopBar = create_top_bar(ui_Screen1);

    ui_cpu_temp_title = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_cpu_temp_title, "CPU TEMP");
    lv_obj_set_style_text_color(ui_cpu_temp_title, lv_color_hex(0x6B7280), 0);
    lv_obj_set_style_text_font(ui_cpu_temp_title, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(ui_cpu_temp_title, 8, 4);

    ui_cpu_temp_val = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_cpu_temp_val, "72°");
    lv_obj_set_style_text_color(ui_cpu_temp_val, lv_color_hex(0xF1F5F9), 0);
    lv_obj_set_style_text_font(ui_cpu_temp_val, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(ui_cpu_temp_val, 8, 20);

    ui_gpu_temp_title = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_gpu_temp_title, "GPU TEMP");
    lv_obj_set_style_text_color(ui_gpu_temp_title, lv_color_hex(0x6B7280), 0);
    lv_obj_set_style_text_font(ui_gpu_temp_title, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_gpu_temp_title, LV_ALIGN_TOP_MID, 0, 4);

    ui_gpu_temp_val = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_gpu_temp_val, "68°");
    lv_obj_set_style_text_color(ui_gpu_temp_val, lv_color_hex(0xB388FF), 0);
    lv_obj_set_style_text_font(ui_gpu_temp_val, &lv_font_montserrat_28, 0);
    lv_obj_align(ui_gpu_temp_val, LV_ALIGN_TOP_MID, 0, 20);

    ui_perf_title = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_perf_title, "PERFORMANCE");
    lv_obj_set_style_text_color(ui_perf_title, lv_color_hex(0x6B7280), 0);
    lv_obj_set_style_text_font(ui_perf_title, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_perf_title, LV_ALIGN_TOP_RIGHT, -8, 4);

    ui_perf_val = lv_label_create(ui_TopBar);
    lv_label_set_text(ui_perf_val, "87%");
    lv_obj_set_style_text_color(ui_perf_val, lv_color_hex(0x00E676), 0);
    lv_obj_set_style_text_font(ui_perf_val, &lv_font_montserrat_28, 0);
    lv_obj_align(ui_perf_val, LV_ALIGN_TOP_RIGHT, -8, 20);

    /* === CPU gauge === */
    ui_cpu_card = create_gauge_card(ui_Screen1, 16, 72);
    ui_cpu_arc = create_arc_gauge(ui_cpu_card, 72, lv_color_hex(0x00E5FF));
    ui_cpu_pct = lv_label_create(ui_cpu_card);
    lv_label_set_text(ui_cpu_pct, "72%");
    lv_obj_set_style_text_color(ui_cpu_pct, lv_color_hex(0xF1F5F9), 0);
    lv_obj_set_style_text_font(ui_cpu_pct, &lv_font_montserrat_28, 0);
    lv_obj_align(ui_cpu_pct, LV_ALIGN_TOP_MID, 0, 32);
    ui_cpu_label = lv_label_create(ui_cpu_card);
    lv_label_set_text(ui_cpu_label, "CPU");
    lv_obj_set_style_text_color(ui_cpu_label, lv_color_hex(0x6B7280), 0);
    lv_obj_set_style_text_font(ui_cpu_label, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_cpu_label, LV_ALIGN_TOP_MID, 0, 62);
    ui_cpu_freq = lv_label_create(ui_cpu_card);
    lv_label_set_text(ui_cpu_freq, "4.2 GHz");
    lv_obj_set_style_text_color(ui_cpu_freq, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(ui_cpu_freq, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_cpu_freq, LV_ALIGN_TOP_MID, 0, 82);

    /* === GPU gauge === */
    ui_gpu_card = create_gauge_card(ui_Screen1, 128, 72);
    ui_gpu_arc = create_arc_gauge(ui_gpu_card, 45, lv_color_hex(0xB388FF));
    ui_gpu_pct = lv_label_create(ui_gpu_card);
    lv_label_set_text(ui_gpu_pct, "45%");
    lv_obj_set_style_text_color(ui_gpu_pct, lv_color_hex(0xF1F5F9), 0);
    lv_obj_set_style_text_font(ui_gpu_pct, &lv_font_montserrat_28, 0);
    lv_obj_align(ui_gpu_pct, LV_ALIGN_TOP_MID, 0, 32);
    ui_gpu_label = lv_label_create(ui_gpu_card);
    lv_label_set_text(ui_gpu_label, "GPU");
    lv_obj_set_style_text_color(ui_gpu_label, lv_color_hex(0x6B7280), 0);
    lv_obj_set_style_text_font(ui_gpu_label, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_gpu_label, LV_ALIGN_TOP_MID, 0, 62);
    ui_gpu_freq = lv_label_create(ui_gpu_card);
    lv_label_set_text(ui_gpu_freq, "1.8 GHz");
    lv_obj_set_style_text_color(ui_gpu_freq, lv_color_hex(0xB388FF), 0);
    lv_obj_set_style_text_font(ui_gpu_freq, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_gpu_freq, LV_ALIGN_TOP_MID, 0, 82);

    /* === Info panel === */
    ui_info_panel = lv_obj_create(ui_Screen1);
    lv_obj_set_pos(ui_info_panel, 8, 192);
    lv_obj_set_size(ui_info_panel, 224, 36);
    lv_obj_set_style_bg_color(ui_info_panel, lv_color_hex(0x141B24), LV_PART_MAIN);
    lv_obj_set_style_radius(ui_info_panel, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_info_panel, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui_info_panel, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_info_panel, 0, LV_PART_MAIN);

    ui_ram_label = lv_label_create(ui_info_panel);
    lv_label_set_text(ui_ram_label, "16.0/32.0 GB");
    lv_obj_set_style_text_color(ui_ram_label, lv_color_hex(0xF1F5F9), 0);
    lv_obj_set_style_text_font(ui_ram_label, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(ui_ram_label, 8, 2);

    ui_ram_bar = lv_bar_create(ui_info_panel);
    lv_obj_set_size(ui_ram_bar, 120, 8);
    lv_bar_set_range(ui_ram_bar, 0, 100);
    lv_bar_set_value(ui_ram_bar, 50, LV_ANIM_OFF);
    lv_obj_set_pos(ui_ram_bar, 8, 22);
    lv_obj_set_style_bg_color(ui_ram_bar, lv_color_hex(0x1E293B), LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_ram_bar, lv_color_hex(0x00E676), LV_PART_INDICATOR);
    lv_obj_set_style_radius(ui_ram_bar, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(ui_ram_bar, 4, LV_PART_INDICATOR);

    ui_net_down = lv_label_create(ui_info_panel);
    lv_label_set_text(ui_net_down, "↓2.3 MB/s");
    lv_obj_set_style_text_color(ui_net_down, lv_color_hex(0x00E5FF), 0);
    lv_obj_set_style_text_font(ui_net_down, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_net_down, LV_ALIGN_TOP_RIGHT, -8, 2);

    ui_net_up = lv_label_create(ui_info_panel);
    lv_label_set_text(ui_net_up, "↑0.5 MB/s");
    lv_obj_set_style_text_color(ui_net_up, lv_color_hex(0xFF9100), 0);
    lv_obj_set_style_text_font(ui_net_up, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_net_up, LV_ALIGN_TOP_RIGHT, -8, 18);

    /* === Status bar === */
    ui_status_bar = lv_obj_create(ui_Screen1);
    lv_obj_set_pos(ui_status_bar, 8, 216);
    lv_obj_set_size(ui_status_bar, 224, 20);
    lv_obj_set_style_bg_color(ui_status_bar, lv_color_hex(0x141B24), LV_PART_MAIN);
    lv_obj_set_style_radius(ui_status_bar, 4, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_status_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui_status_bar, lv_color_hex(0x00E5FF), LV_PART_MAIN);
    lv_obj_set_style_border_opa(ui_status_bar, 25, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_status_bar, 0, LV_PART_MAIN);

    ui_status_left = lv_label_create(ui_status_bar);
    lv_label_set_text(ui_status_left, "PC CONNECTED");
    lv_obj_set_style_text_color(ui_status_left, lv_color_hex(0x6B7280), 0);
    lv_obj_set_style_text_font(ui_status_left, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(ui_status_left, 8, 2);

    ui_status_mid = lv_label_create(ui_status_bar);
    lv_label_set_text(ui_status_mid, "DHT11 26.5°C 45%");
    lv_obj_set_style_text_color(ui_status_mid, lv_color_hex(0x6B7280), 0);
    lv_obj_set_style_text_font(ui_status_mid, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_status_mid, LV_ALIGN_TOP_MID, 0, 2);

    ui_status_right = lv_label_create(ui_status_bar);
    lv_label_set_text(ui_status_right, "GAME BOOST v1");
    lv_obj_set_style_text_color(ui_status_right, lv_color_hex(0x6B7280), 0);
    lv_obj_set_style_text_font(ui_status_right, &lv_font_montserrat_14, 0);
    lv_obj_align(ui_status_right, LV_ALIGN_TOP_RIGHT, -8, 2);
}

void ui_init(void) {
    ui_Screen1_screen_init();
    lv_disp_load_scr(ui_Screen1);
}
