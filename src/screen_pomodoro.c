#include "stacks_app.h"
#include "anim_utils.h"
#include <stdio.h>

typedef enum { POMO_WORKTIME = 0, POMO_LOOPS } pomo_state_t;

static pomo_state_t  g_state = POMO_WORKTIME;
static int           g_work  = 25;
static int           g_loops = 10;

static lv_obj_t *g_work_pill,  *g_work_lbl;
static lv_obj_t *g_loops_pill, *g_loops_lbl;
static lv_obj_t *g_sub_lbl;
static lv_obj_t *g_dt_lbl, *g_play, *g_cnt, *g_tsk;

static void pomo_refresh(void)
{
    char buf[24];

    if (g_state == POMO_WORKTIME) {
        lv_obj_set_style_bg_color(g_work_pill,  CLR_BTN,  0);
        lv_obj_set_style_bg_color(g_loops_pill, CLR_INNER, 0);
        snprintf(buf, sizeof(buf), "< %d >", g_work);
        lv_label_set_text(g_work_lbl, buf);
        lv_obj_set_style_text_color(g_work_lbl, CLR_WHITE, 0);
        snprintf(buf, sizeof(buf), "%d", g_loops);
        lv_label_set_text(g_loops_lbl, buf);
        lv_obj_set_style_text_color(g_loops_lbl, CLR_GRAY, 0);
        lv_label_set_text(g_sub_lbl, "Work time (min)");
    } else {
        lv_obj_set_style_bg_color(g_loops_pill, CLR_BTN,  0);
        lv_obj_set_style_bg_color(g_work_pill,  CLR_INNER, 0);
        snprintf(buf, sizeof(buf), "< %d >", g_loops);
        lv_label_set_text(g_loops_lbl, buf);
        lv_obj_set_style_text_color(g_loops_lbl, CLR_WHITE, 0);
        snprintf(buf, sizeof(buf), "%d", g_work);
        lv_label_set_text(g_work_lbl, buf);
        lv_obj_set_style_text_color(g_work_lbl, CLR_GRAY, 0);
        lv_label_set_text(g_sub_lbl, "Loop count");
    }
}

static void pomo_key_cb(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    if      (key == LV_KEY_ENTER || key == ' ')
        g_state = !g_state;
    else if (key == LV_KEY_UP) {
        if (g_state == POMO_WORKTIME) g_work  = LV_MIN(g_work  + 5, 60);
        else                          g_loops = LV_MIN(g_loops + 1, 20);
    } else if (key == LV_KEY_DOWN) {
        if (g_state == POMO_WORKTIME) g_work  = LV_MAX(g_work  - 5,  5);
        else                          g_loops = LV_MAX(g_loops - 1,  1);
    }
    pomo_refresh();
}

static lv_obj_t *make_pill(lv_obj_t *parent, int32_t w, int32_t h, lv_color_t bg)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(o, bg, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_outline_width(o, 0, 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return o;
}

lv_obj_t *screen_pomodoro_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_opa(scr, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* Datetime */
    char dt[40]; stacks_get_datetime(dt, sizeof(dt));
    g_dt_lbl = lv_label_create(scr);
    lv_label_set_text(g_dt_lbl, dt);
    lv_obj_set_style_text_font(g_dt_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_dt_lbl, CLR_GRAY, 0);
    lv_obj_align(g_dt_lbl, LV_ALIGN_CENTER, 0, -132);

    /* Loops pill */
    g_loops_pill = make_pill(scr, 170, 44, CLR_INNER);
    lv_obj_align(g_loops_pill, LV_ALIGN_CENTER, 0, -72);
    g_loops_lbl = lv_label_create(g_loops_pill);
    lv_label_set_text(g_loops_lbl, "10");
    lv_obj_set_style_text_font(g_loops_lbl, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(g_loops_lbl, CLR_GRAY, 0);
    lv_obj_center(g_loops_lbl);

    /* Work-time pill */
    g_work_pill = make_pill(scr, 200, 80, CLR_BTN);
    lv_obj_align(g_work_pill, LV_ALIGN_CENTER, 0, -4);
    g_work_lbl = lv_label_create(g_work_pill);
    lv_label_set_text(g_work_lbl, "< 25 >");
    lv_obj_set_style_text_font(g_work_lbl, &lv_font_montserrat_38, 0);
    lv_obj_set_style_text_color(g_work_lbl, CLR_WHITE, 0);
    lv_obj_center(g_work_lbl);

    /* Sub label */
    g_sub_lbl = lv_label_create(scr);
    lv_label_set_text(g_sub_lbl, "Work time (min)");
    lv_obj_set_style_text_font(g_sub_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_sub_lbl, CLR_GRAY, 0);
    lv_obj_align(g_sub_lbl, LV_ALIGN_CENTER, 0, 46);

    /* Play button */
    g_play = make_pill(scr, 160, 48, CLR_BTN);
    lv_obj_align(g_play, LV_ALIGN_CENTER, 0, 96);
    lv_obj_t *play_lbl = lv_label_create(g_play);
    lv_label_set_text(play_lbl, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(play_lbl, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(play_lbl, CLR_WHITE, 0);
    lv_obj_center(play_lbl);

    /* Task count */
    g_cnt = lv_label_create(scr);
    lv_label_set_text(g_cnt, "12");
    lv_obj_set_style_text_font(g_cnt, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_cnt, CLR_RED, 0);
    lv_obj_align(g_cnt, LV_ALIGN_CENTER, -12, 150);

    g_tsk = lv_label_create(scr);
    lv_label_set_text(g_tsk, "Tasks");
    lv_obj_set_style_text_font(g_tsk, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(g_tsk, CLR_GRAY, 0);
    lv_obj_align(g_tsk, LV_ALIGN_CENTER, 0, 198);

    lv_obj_add_event_cb(scr, pomo_key_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    return scr;
}

/* Entrance animation — mirrors the Svelte firmware's pageIn from /clock.
 *
 * dir=+1 (from Pomodoro's perspective, arriving from left / Clock exited right):
 *   All UI panels slide in from the left (−220 px → 0).
 * dir=−1 (arriving from right / clock is to the right):
 *   Same but panels slide in from the right (+220 px → 0).
 *
 * Task count (bottom section) comes from below regardless of direction,
 * matching the translateY(130px) → 0 in the firmware.
 */
void screen_pomodoro_anim_in(int dir)
{
    int32_t  off  = (int32_t)(dir * 220);
    uint32_t base = SCR_SLIDE_MS;   /* start after screen lands */

    /* Pre-set so elements are already at offset when screen arrives */
    lv_obj_set_style_translate_x(g_dt_lbl,     off, 0);
    lv_obj_set_style_translate_x(g_loops_pill,  off, 0);
    lv_obj_set_style_translate_x(g_work_pill,   off, 0);
    lv_obj_set_style_translate_x(g_sub_lbl,     off, 0);
    lv_obj_set_style_translate_x(g_play,        off, 0);
    lv_obj_set_style_translate_y(g_cnt,         130, 0);
    lv_obj_set_style_translate_y(g_tsk,         130, 0);

    stacks_tx_anim(g_dt_lbl,     off, 0, 360, base);
    stacks_tx_anim(g_loops_pill,  off, 0, 380, base + 20);
    stacks_tx_anim(g_work_pill,   off, 0, 400, base);
    stacks_tx_anim(g_sub_lbl,     off, 0, 360, base + 50);
    stacks_tx_anim(g_play,        off, 0, 380, base + 30);
    stacks_ty_anim(g_cnt,         130, 0, 400, base);
    stacks_ty_anim(g_tsk,         130, 0, 400, base + 40);
}
