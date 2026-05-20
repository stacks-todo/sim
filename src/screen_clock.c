#include "stacks_app.h"
#include "anim_utils.h"
#include <math.h>
#include <time.h>
#include <stdio.h>

#define CLOCK_NUM_R  168
#define HOUR_LEN      76
#define MIN_LEN      110
#define SEC_LEN      118

static lv_obj_t *g_time_lbl, *g_date_lbl;
static lv_obj_t *g_hour_line, *g_min_line, *g_sec_line;
static lv_obj_t *g_digit_lbls[12];
static lv_obj_t *g_task_cnt, *g_task_tsk;

/* Static point arrays — must stay alive as long as the lines exist */
static lv_point_precise_t g_hour_pts[2];
static lv_point_precise_t g_min_pts[2];
static lv_point_precise_t g_sec_pts[2];

static void update_hands(int hh, int mm, int ss)
{
    float ha = (((hh % 12) * 30.0f + mm * 0.5f) - 90.0f) * (float)M_PI / 180.0f;
    float ma = ((mm * 6.0f  + ss * 0.1f)         - 90.0f) * (float)M_PI / 180.0f;
    float sa = ((ss * 6.0f)                       - 90.0f) * (float)M_PI / 180.0f;

    g_hour_pts[0].x = DISP_CX; g_hour_pts[0].y = DISP_CY;
    g_hour_pts[1].x = DISP_CX + HOUR_LEN * cosf(ha);
    g_hour_pts[1].y = DISP_CY + HOUR_LEN * sinf(ha);

    g_min_pts[0].x  = DISP_CX; g_min_pts[0].y  = DISP_CY;
    g_min_pts[1].x  = DISP_CX + MIN_LEN * cosf(ma);
    g_min_pts[1].y  = DISP_CY + MIN_LEN * sinf(ma);

    g_sec_pts[0].x  = DISP_CX; g_sec_pts[0].y  = DISP_CY;
    g_sec_pts[1].x  = DISP_CX + SEC_LEN * cosf(sa);
    g_sec_pts[1].y  = DISP_CY + SEC_LEN * sinf(sa);

    lv_line_set_points_mutable(g_hour_line, g_hour_pts, 2);
    lv_line_set_points_mutable(g_min_line,  g_min_pts,  2);
    lv_line_set_points_mutable(g_sec_line,  g_sec_pts,  2);
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

    /* Hour digits 1–12 */
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
        lv_obj_remove_flag(lbl, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_update_layout(lbl);
        lv_obj_set_pos(lbl,
                       x - lv_obj_get_width(lbl)  / 2,
                       y - lv_obj_get_height(lbl) / 2);
        g_digit_lbls[i] = lbl;
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
    lv_obj_remove_flag(tick, LV_OBJ_FLAG_CLICKABLE);

    /* Inner black overlay */
    stacks_make_circle(scr, 240, CLR_INNER);

    /* Time label */
    g_time_lbl = lv_label_create(scr);
    lv_label_set_text(g_time_lbl, "00:00");
    lv_obj_set_style_text_font(g_time_lbl, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(g_time_lbl, CLR_WHITE, 0);
    lv_obj_remove_flag(g_time_lbl, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(g_time_lbl, LV_ALIGN_CENTER, 0, -52);

    /* Date label */
    g_date_lbl = lv_label_create(scr);
    lv_label_set_text(g_date_lbl, "---");
    lv_obj_set_style_text_font(g_date_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_date_lbl, CLR_GRAY, 0);
    lv_obj_remove_flag(g_date_lbl, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(g_date_lbl, LV_ALIGN_CENTER, 0, -28);

    /* Task count */
    g_task_cnt = lv_label_create(scr);
    lv_label_set_text(g_task_cnt, "12");
    lv_obj_set_style_text_font(g_task_cnt, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_task_cnt, CLR_RED, 0);
    lv_obj_remove_flag(g_task_cnt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(g_task_cnt, LV_ALIGN_CENTER, -16, 12);

    g_task_tsk = lv_label_create(scr);
    lv_label_set_text(g_task_tsk, "Tasks");
    lv_obj_set_style_text_font(g_task_tsk, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(g_task_tsk, CLR_GRAY, 0);
    lv_obj_remove_flag(g_task_tsk, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(g_task_tsk, LV_ALIGN_CENTER, 0, 60);

    /* Clock hands — lv_line (no transform_rotation, no parent-chain traversal) */
    g_hour_line = lv_line_create(scr);
    lv_obj_set_style_line_color(g_hour_line, CLR_WHITE, 0);
    lv_obj_set_style_line_width(g_hour_line, 6, 0);
    lv_obj_set_style_line_rounded(g_hour_line, true, 0);
    lv_obj_remove_flag(g_hour_line, LV_OBJ_FLAG_CLICKABLE);

    g_min_line = lv_line_create(scr);
    lv_obj_set_style_line_color(g_min_line, CLR_WHITE, 0);
    lv_obj_set_style_line_width(g_min_line, 4, 0);
    lv_obj_set_style_line_rounded(g_min_line, true, 0);
    lv_obj_remove_flag(g_min_line, LV_OBJ_FLAG_CLICKABLE);

    g_sec_line = lv_line_create(scr);
    lv_obj_set_style_line_color(g_sec_line, CLR_RED, 0);
    lv_obj_set_style_line_width(g_sec_line, 2, 0);
    lv_obj_set_style_line_rounded(g_sec_line, true, 0);
    lv_obj_remove_flag(g_sec_line, LV_OBJ_FLAG_CLICKABLE);

    /* Center cap */
    stacks_make_circle(scr, 10, CLR_RED);

    /* Set initial hand positions then start 1-second timer */
    clock_tick_cb(NULL);
    lv_timer_create(clock_tick_cb, 1000, NULL);
    return scr;
}

/* Entrance animation — called by stacks_app.c before lv_screen_load_anim.
 *
 * From Pomodoro (dir=+1): numerals + clock wrap slide in from the right.
 * From Stack   (dir=-1): same but from the left.
 *
 * Task count slides up from below in both cases (mirrors the original
 * translateY(130px) → 0 in the Svelte firmware).
 */
void screen_clock_anim_in(int dir)
{
    int32_t  off  = (int32_t)(dir * 220);
    uint32_t base = SCR_SLIDE_MS;   /* start after screen lands */

    /* Pre-set initial offset so elements are already positioned when the
     * screen arrives; animations then pull them to their final places. */
    for (int i = 0; i < 12; i++)
        lv_obj_set_style_translate_x(g_digit_lbls[i], off, 0);
    lv_obj_set_style_translate_x(g_time_lbl, off, 0);
    lv_obj_set_style_translate_x(g_date_lbl, off, 0);
    lv_obj_set_style_translate_y(g_task_cnt, 130, 0);
    lv_obj_set_style_translate_y(g_task_tsk, 130, 0);

    /* Stagger the 12 digit labels (12 ms apart) */
    for (int i = 0; i < 12; i++)
        stacks_tx_anim(g_digit_lbls[i], off, 0, 380, base + (uint32_t)(i * 12));

    /* Time and date */
    stacks_tx_anim(g_time_lbl, off, 0, 380, base + 20);
    stacks_tx_anim(g_date_lbl, off, 0, 380, base + 35);

    /* Task count slides up from below */
    stacks_ty_anim(g_task_cnt, 130, 0, 400, base);
    stacks_ty_anim(g_task_tsk, 130, 0, 400, base + 30);
}
