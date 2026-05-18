#include "stacks_app.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

#define CLOCK_NUM_R  168
#define HOUR_LEN      70
#define MIN_LEN      115

static lv_obj_t            *g_time_lbl, *g_date_lbl;
static lv_obj_t            *g_hour_line, *g_min_line;
static lv_point_precise_t   g_hour_pts[2], g_min_pts[2];

static void update_hands(int h, int m, int s)
{
    float ha = ((h % 12) * 30.0f + m * 0.5f  - 90.0f) * (float)M_PI / 180.0f;
    float ma = (m * 6.0f          + s * 0.1f  - 90.0f) * (float)M_PI / 180.0f;

    g_hour_pts[0] = (lv_point_precise_t){ DISP_CX, DISP_CY };
    g_hour_pts[1] = (lv_point_precise_t){ DISP_CX + HOUR_LEN * cosf(ha),
                                          DISP_CY + HOUR_LEN * sinf(ha) };
    lv_line_set_points_mutable(g_hour_line, g_hour_pts, 2);

    g_min_pts[0] = (lv_point_precise_t){ DISP_CX, DISP_CY };
    g_min_pts[1] = (lv_point_precise_t){ DISP_CX + MIN_LEN * cosf(ma),
                                         DISP_CY + MIN_LEN * sinf(ma) };
    lv_line_set_points_mutable(g_min_line, g_min_pts, 2);
}

static void clock_tick_cb(lv_timer_t *t)
{
    (void)t;
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char buf[32];

    snprintf(buf, sizeof(buf), "%02d:%02d", tm->tm_hour, tm->tm_min);
    lv_label_set_text(g_time_lbl, buf);

    strftime(buf, sizeof(buf), "%b %d (%a)", tm);
    lv_label_set_text(g_date_lbl, buf);

    update_hands(tm->tm_hour, tm->tm_min, tm->tm_sec);
}

lv_obj_t *screen_clock_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    stacks_create_bg(scr);

    /* Hour digits 1–12 on the gray ring */
    static const char *DIGITS[] = {
        "1","2","3","4","5","6","7","8","9","10","11","12"
    };
    for (int i = 0; i < 12; i++) {
        float rad = ((i + 1) * 30.0f - 90.0f) * (float)M_PI / 180.0f;
        int32_t x = (int32_t)(DISP_CX + CLOCK_NUM_R * cosf(rad));
        int32_t y = (int32_t)(DISP_CY + CLOCK_NUM_R * sinf(rad));

        lv_obj_t *lbl = lv_label_create(scr);
        lv_label_set_text(lbl, DIGITS[i]);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(lbl, CLR_GRAY, 0);
        lv_obj_update_layout(lbl);
        lv_obj_set_pos(lbl,
                       x - lv_obj_get_width(lbl)  / 2,
                       y - lv_obj_get_height(lbl) / 2);
    }

    /* 12-o'clock tick */
    static lv_point_precise_t tick_pts[2] = {
        { DISP_CX, DISP_CY - RING_MID_D / 2 + 4  },
        { DISP_CX, DISP_CY - RING_MID_D / 2 + 20 }
    };
    lv_obj_t *tick = lv_line_create(scr);
    lv_line_set_points(tick, tick_pts, 2);
    lv_obj_set_style_line_color(tick, CLR_GRAY_LIGHT, 0);
    lv_obj_set_style_line_width(tick, 3, 0);
    lv_obj_set_style_line_rounded(tick, true, 0);

    /* Inner black overlay */
    stacks_make_circle(scr, 240, CLR_INNER);

    /* Time */
    g_time_lbl = lv_label_create(scr);
    lv_label_set_text(g_time_lbl, "00:00");
    lv_obj_set_style_text_font(g_time_lbl, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(g_time_lbl, CLR_WHITE, 0);
    lv_obj_align(g_time_lbl, LV_ALIGN_CENTER, 0, -52);

    /* Date */
    g_date_lbl = lv_label_create(scr);
    lv_label_set_text(g_date_lbl, "---");
    lv_obj_set_style_text_font(g_date_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_date_lbl, CLR_GRAY, 0);
    lv_obj_align(g_date_lbl, LV_ALIGN_CENTER, 0, -28);

    /* Task count */
    lv_obj_t *cnt = lv_label_create(scr);
    lv_label_set_text(cnt, "12");
    lv_obj_set_style_text_font(cnt, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(cnt, CLR_RED, 0);
    lv_obj_align(cnt, LV_ALIGN_CENTER, -16, 12);

    lv_obj_t *tsk = lv_label_create(scr);
    lv_label_set_text(tsk, "Tasks");
    lv_obj_set_style_text_font(tsk, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(tsk, CLR_GRAY, 0);
    lv_obj_align(tsk, LV_ALIGN_CENTER, 0, 60);

    /* Clock hands */
    g_hour_line = lv_line_create(scr);
    lv_obj_set_style_line_color(g_hour_line, CLR_WHITE, 0);
    lv_obj_set_style_line_width(g_hour_line, 5, 0);
    lv_obj_set_style_line_rounded(g_hour_line, true, 0);

    g_min_line = lv_line_create(scr);
    lv_obj_set_style_line_color(g_min_line, CLR_WHITE, 0);
    lv_obj_set_style_line_width(g_min_line, 3, 0);
    lv_obj_set_style_line_rounded(g_min_line, true, 0);

    stacks_make_circle(scr, 12, CLR_WHITE);   /* center dot */

    clock_tick_cb(NULL);
    lv_timer_create(clock_tick_cb, 1000, NULL);
    return scr;
}
