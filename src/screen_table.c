#include "stacks_app.h"
#include <stdio.h>

#define N_TASKS   12
#define CARD_W    272
#define CARD_H     76
#define CARD_GAP    8
#define LIST_H    230   /* visible area for the list */

typedef struct { const char *title, *due; } task_t;

static const task_t TASKS[N_TASKS] = {
    { "Video Art Essay",   "3 days"   },
    { "Backlog Games",     "No due"   },
    { "Read LVGL Docs",    "Today"    },
    { "Design Review",     "2 days"   },
    { "Morning Run",       "Tomorrow" },
    { "Buy Groceries",     "No due"   },
    { "Refactor UI code",  "4 days"   },
    { "Weekly Report",     "5 days"   },
    { "Update CLAUDE.md",  "Today"    },
    { "Standup Notes",     "No due"   },
    { "Check Figma Specs", "2 days"   },
    { "Submit PR #42",     "Tomorrow" },
};

static lv_color_t dot_color(int i)
{
    switch (i % 4) {
        case 0: return CLR_ORANGE;
        case 1: return CLR_BLUE;
        case 2: return CLR_RED;
        default: return CLR_GRAY;
    }
}

static lv_obj_t *g_list;

static void table_key_cb(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    int32_t step = CARD_H + CARD_GAP;
    if      (key == LV_KEY_UP)
        lv_obj_scroll_by(g_list, 0, -step, LV_ANIM_ON);
    else if (key == LV_KEY_DOWN)
        lv_obj_scroll_by(g_list, 0,  step, LV_ANIM_ON);
}

lv_obj_t *screen_table_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    stacks_create_bg(scr);

    /* ── header ──────────────────────────────────────────── */
    char dt[40]; stacks_get_datetime(dt, sizeof(dt));
    lv_obj_t *dt_lbl = lv_label_create(scr);
    lv_label_set_text(dt_lbl, dt);
    lv_obj_set_style_text_font(dt_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(dt_lbl, CLR_GRAY, 0);
    lv_obj_align(dt_lbl, LV_ALIGN_CENTER, 0, -148);

    lv_obj_t *cnt = lv_label_create(scr);
    lv_label_set_text(cnt, "12");
    lv_obj_set_style_text_font(cnt, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(cnt, CLR_RED, 0);
    lv_obj_align(cnt, LV_ALIGN_CENTER, -12, -110);

    lv_obj_t *tsk = lv_label_create(scr);
    lv_label_set_text(tsk, "Tasks");
    lv_obj_set_style_text_font(tsk, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(tsk, CLR_GRAY, 0);
    lv_obj_align(tsk, LV_ALIGN_CENTER, 0, -72);

    /* ── scrollable list ─────────────────────────────────── */
    g_list = lv_obj_create(scr);
    lv_obj_set_size(g_list, CARD_W + 12, LIST_H);
    lv_obj_align(g_list, LV_ALIGN_CENTER, 0, 42);
    lv_obj_set_style_bg_opa(g_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_list, 0, 0);
    lv_obj_set_style_outline_width(g_list, 0, 0);
    lv_obj_set_style_pad_all(g_list, 0, 0);
    lv_obj_set_style_pad_row(g_list, CARD_GAP, 0);
    lv_obj_set_flex_flow(g_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(g_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(g_list, LV_SCROLLBAR_MODE_ACTIVE);
    lv_obj_set_style_width(g_list, 4, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(g_list, CLR_GRAY, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(g_list, LV_OPA_COVER, LV_PART_SCROLLBAR);
    lv_obj_set_style_radius(g_list, 2, LV_PART_SCROLLBAR);

    for (int i = 0; i < N_TASKS; i++) {
        lv_obj_t *card = lv_obj_create(g_list);
        lv_obj_set_size(card, CARD_W, CARD_H);
        lv_obj_set_style_radius(card, 14, 0);
        lv_obj_set_style_bg_color(card, CLR_CARD, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_set_style_outline_width(card, 0, 0);
        lv_obj_set_style_pad_left(card, 16, 0);
        lv_obj_set_style_pad_top(card, 12, 0);
        lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *title = lv_label_create(card);
        lv_label_set_text(title, TASKS[i].title);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(title, CLR_WHITE, 0);
        lv_label_set_long_mode(title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(title, CARD_W - 32);
        lv_obj_set_pos(title, 0, 0);

        lv_obj_t *dot = lv_obj_create(card);
        lv_obj_set_size(dot, 8, 8);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, dot_color(i), 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_set_pos(dot, 0, 40);

        lv_obj_t *due = lv_label_create(card);
        lv_label_set_text(due, TASKS[i].due);
        lv_obj_set_style_text_font(due, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(due, CLR_GRAY, 0);
        lv_obj_set_pos(due, 14, 36);
    }

    lv_obj_add_event_cb(scr, table_key_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    return scr;
}
