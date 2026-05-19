#include "stacks_app.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

#define CLOCK_NUM_R  168
#define HOUR_LEN      76   /* hand length (px) */
#define MIN_LEN      110
#define SEC_LEN      122
#define HOUR_W         6
#define MIN_W          4
#define SEC_W          2

static lv_obj_t *g_time_lbl, *g_date_lbl;
static lv_obj_t *g_hour_hand, *g_min_hand, *g_sec_hand;

/*
 * Each hand is a thin rounded rectangle.
 * Pivot is set to its bottom-center so it rotates around the clock center.
 * transform_rotation is in 0.1-degree units, 0 = pointing straight up (12 o'clock).
 */
static lv_obj_t *make_hand(lv_obj_t *scr, int32_t len, int32_t w, lv_color_t color)
{
    lv_obj_t *h = lv_obj_create(scr);
    lv_obj_set_size(h, w, len);
    lv_obj_set_style_radius(h, w / 2, 0);
    lv_obj_set_style_bg_color(h, color, 0);
    lv_obj_set_style_bg_opa(h, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(h, 0, 0);
    lv_obj_set_style_outline_width(h, 0, 0);
    lv_obj_remove_flag(h, LV_OBJ_FLAG_SCROLLABLE);

    /* pivot at bottom-center of the rectangle = clock center */
    lv_obj_set_style_transform_pivot_x(h, w / 2, 0);
    lv_obj_set_style_transform_pivot_y(h, len,   0);

    /* place so that pivot lands exactly on (DISP_CX, DISP_CY) */
    lv_obj_set_pos(h, DISP_CX - w / 2, DISP_CY - len);
    return h;
}

static void update_hands(int hh, int mm, int ss)
{
    /* angle in 0.1° units clockwise from 12 o'clock */
    int32_t ha = (int32_t)(((hh % 12) * 30.0f + mm * 0.5f) * 10.0f);
    int32_t ma = (int32_t)((mm        *  6.0f + ss * 0.1f) * 10.0f);
    int32_t sa = ss * 60;   /* 6° per second × 10 = 60 */
    lv_obj_set_style_transform_rotation(g_hour_hand, ha, 0);
    lv_obj_set_style_transform_rotation(g_min_hand,  ma, 0);
    lv_obj_set_style_transform_rotation(g_sec_hand,  sa, 0);
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

    /* 12-o'clock tick mark */
    static lv_point_precise_t tick_pts[2] = {
        { DISP_CX, DISP_CY - RING_MID_D / 2 + 4  },
        { DISP_CX, DISP_CY - RING_MID_D / 2 + 20 }
    };
    lv_obj_t *tick = lv_line_create(scr);
    lv_line_set_points(tick, tick_pts, 2);
    lv_obj_set_style_line_color(tick, CLR_GRAY_LIGHT, 0);
    lv_obj_set_style_line_width(tick, 3, 0);
    lv_obj_set_style_line_rounded(tick, true, 0);

    /* Inner black overlay (covers the digit ring interior) */
    stacks_make_circle(scr, 240, CLR_INNER);

    /* Time label */
    g_time_lbl = lv_label_create(scr);
    lv_label_set_text(g_time_lbl, "00:00");
    lv_obj_set_style_text_font(g_time_lbl, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(g_time_lbl, CLR_WHITE, 0);
    lv_obj_align(g_time_lbl, LV_ALIGN_CENTER, 0, -52);

    /* Date label */
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

    /* Clock hands (created after overlay so they draw on top) */
    g_hour_hand = make_hand(scr, HOUR_LEN, HOUR_W, CLR_WHITE);
    g_min_hand  = make_hand(scr, MIN_LEN,  MIN_W,  CLR_WHITE);
    g_sec_hand  = make_hand(scr, SEC_LEN,  SEC_W,  CLR_RED);

    /* Center cap */
    stacks_make_circle(scr, 10, CLR_RED);

    /* Initial draw + recurring timer */
    clock_tick_cb(NULL);
    lv_timer_create(clock_tick_cb, 1000, NULL);
    return scr;
}
