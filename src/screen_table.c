#include "stacks_app.h"
#include "anim_utils.h"
#include <stdio.h>

/* ── Task data ───────────────────────────────────────────────── */

#define N_TASKS  12

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

static lv_color_t task_color(int i)
{
    switch (i % 3) {
        case 0:  return CLR_RED;
        case 1:  return CLR_ORANGE;
        default: return CLR_BLUE;
    }
}

/* ── Card layout constants ───────────────────────────────────── */

#define CARD_W   278
#define CARD_H   158
/* Vertical travel for card-to-card transition (px) */
#define SLIDE_Y  310

/* ── Widget references ───────────────────────────────────────── */

/* Double-buffered cards: one is on-screen, the other waits off-screen. */
static lv_obj_t *g_wrap[2];        /* card containers */
static lv_obj_t *g_dot[2];         /* colored accent dot */
static lv_obj_t *g_due[2];         /* due-date label */
static lv_obj_t *g_title[2];       /* task title */

static lv_obj_t *g_tbl_cnt;        /* large task-count number */
static lv_obj_t *g_tbl_tsk;        /* "Tasks" sub-label */
static lv_obj_t *g_idx_lbl;        /* "3 / 12" pager indicator */

static int g_buf     = 0;          /* which buffer is currently visible */
static int g_cur_idx = 0;          /* index of the displayed task */

/* ── Content helpers ─────────────────────────────────────────── */

static void set_card(int buf, int idx)
{
    lv_color_t col = task_color(idx);

    lv_obj_set_style_bg_color(g_wrap[buf], CLR_CARD, 0);
    lv_obj_set_style_border_color(g_wrap[buf], col, 0);

    lv_obj_set_style_bg_color(g_dot[buf], col, 0);

    lv_label_set_text(g_due[buf],   TASKS[idx].due);
    lv_label_set_text(g_title[buf], TASKS[idx].title);
}

static void update_idx(void)
{
    char buf[12];
    snprintf(buf, sizeof(buf), "%d / %d", g_cur_idx + 1, N_TASKS);
    lv_label_set_text(g_idx_lbl, buf);
}

/* ── Card factory ────────────────────────────────────────────── */

static void build_card(lv_obj_t *parent, int b)
{
    /* Container */
    g_wrap[b] = lv_obj_create(parent);
    lv_obj_set_size(g_wrap[b], CARD_W, CARD_H);
    lv_obj_align(g_wrap[b], LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_radius(g_wrap[b], 22, 0);
    lv_obj_set_style_bg_color(g_wrap[b], CLR_CARD, 0);
    lv_obj_set_style_bg_opa(g_wrap[b], LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_wrap[b], 2, 0);
    lv_obj_set_style_border_opa(g_wrap[b], LV_OPA_50, 0);
    lv_obj_set_style_pad_left(g_wrap[b], 22, 0);
    lv_obj_set_style_pad_right(g_wrap[b], 22, 0);
    lv_obj_set_style_pad_top(g_wrap[b], 18, 0);
    lv_obj_set_style_pad_bottom(g_wrap[b], 18, 0);
    lv_obj_remove_flag(g_wrap[b], LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    /* Accent dot */
    g_dot[b] = lv_obj_create(g_wrap[b]);
    lv_obj_set_size(g_dot[b], 10, 10);
    lv_obj_set_style_radius(g_dot[b], LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(g_dot[b], CLR_RED, 0);
    lv_obj_set_style_bg_opa(g_dot[b], LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_dot[b], 0, 0);
    lv_obj_set_pos(g_dot[b], 0, 2);
    lv_obj_remove_flag(g_dot[b], LV_OBJ_FLAG_CLICKABLE);

    /* Due date */
    g_due[b] = lv_label_create(g_wrap[b]);
    lv_label_set_text(g_due[b], "---");
    lv_obj_set_style_text_font(g_due[b], &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_due[b], CLR_GRAY_LIGHT, 0);
    lv_obj_set_pos(g_due[b], 18, 0);
    lv_obj_remove_flag(g_due[b], LV_OBJ_FLAG_CLICKABLE);

    /* Task title */
    g_title[b] = lv_label_create(g_wrap[b]);
    lv_label_set_text(g_title[b], "---");
    lv_obj_set_style_text_font(g_title[b], &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(g_title[b], CLR_WHITE, 0);
    lv_label_set_long_mode(g_title[b], LV_LABEL_LONG_WRAP);
    lv_obj_set_width(g_title[b], CARD_W - 44);
    lv_obj_align(g_title[b], LV_ALIGN_LEFT_MID, 0, 14);
    lv_obj_remove_flag(g_title[b], LV_OBJ_FLAG_CLICKABLE);
}

/* ── Card navigation ─────────────────────────────────────────── */

/*  dir = +1 → next task  (↓ key: current slides up, next arrives from below)
 *  dir = -1 → prev task  (↑ key: current slides down, prev arrives from above) */
static void table_navigate(int dir)
{
    int new_idx = g_cur_idx + dir;
    if (new_idx < 0 || new_idx >= N_TASKS) return;

    int next = 1 - g_buf;

    /* Prepare the off-screen card with new content */
    set_card(next, new_idx);
    int32_t from_y = (int32_t)(dir * SLIDE_Y);
    lv_obj_set_style_translate_y(g_wrap[next], from_y, 0);
    lv_obj_set_style_opa(g_wrap[next], LV_OPA_COVER, 0);

    /* Slide current card out (EASE_IN) */
    stacks_ty_anim_out(g_wrap[g_buf], 0, -from_y, 220, 0);

    /* Slide new card in (EASE_OUT) */
    stacks_ty_anim(g_wrap[next], from_y, 0, 300, 0);

    g_cur_idx = new_idx;
    g_buf     = next;
    update_idx();
}

/* ── Key handler ─────────────────────────────────────────────── */

static void table_key_cb(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    if      (key == LV_KEY_UP)   table_navigate(-1);
    else if (key == LV_KEY_DOWN) table_navigate(+1);
}

/* ── Screen create ───────────────────────────────────────────── */

lv_obj_t *screen_table_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_opa(scr, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* Date */
    char dt[40];
    stacks_get_datetime(dt, sizeof(dt));
    lv_obj_t *dt_lbl = lv_label_create(scr);
    lv_label_set_text(dt_lbl, dt);
    lv_obj_set_style_text_font(dt_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(dt_lbl, CLR_GRAY, 0);
    lv_obj_align(dt_lbl, LV_ALIGN_CENTER, 0, -148);
    lv_obj_remove_flag(dt_lbl, LV_OBJ_FLAG_CLICKABLE);

    /* Task count — large number at top */
    g_tbl_cnt = lv_label_create(scr);
    lv_label_set_text(g_tbl_cnt, "12");
    lv_obj_set_style_text_font(g_tbl_cnt, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_tbl_cnt, CLR_RED, 0);
    lv_obj_align(g_tbl_cnt, LV_ALIGN_CENTER, -12, -108);
    lv_obj_remove_flag(g_tbl_cnt, LV_OBJ_FLAG_CLICKABLE);

    g_tbl_tsk = lv_label_create(scr);
    lv_label_set_text(g_tbl_tsk, "Tasks");
    lv_obj_set_style_text_font(g_tbl_tsk, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(g_tbl_tsk, CLR_GRAY, 0);
    lv_obj_align(g_tbl_tsk, LV_ALIGN_CENTER, 0, -68);
    lv_obj_remove_flag(g_tbl_tsk, LV_OBJ_FLAG_CLICKABLE);

    /* Double-buffered card pair */
    build_card(scr, 0);
    build_card(scr, 1);

    /* Pager indicator: "1 / 12" */
    g_idx_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(g_idx_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_idx_lbl, CLR_GRAY, 0);
    lv_obj_align(g_idx_lbl, LV_ALIGN_CENTER, 0, 108);
    lv_obj_remove_flag(g_idx_lbl, LV_OBJ_FLAG_CLICKABLE);

    /* Initialise card 0 with first task; park card 1 off-screen */
    g_buf     = 0;
    g_cur_idx = 0;
    set_card(0, 0);
    update_idx();
    lv_obj_set_style_translate_y(g_wrap[1], SLIDE_Y, 0);

    lv_obj_add_event_cb(scr, table_key_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    return scr;
}

/* ── Entrance animation ──────────────────────────────────────── */

/*  Mirrors the STACKS firmware animateIn for the Table page:
 *    - Task-count header drops from y+200 with EASE_OUT (0.6 s in firmware)
 *    - Active card fades + rises from y+10 (card stagger approximated as single card) */
void screen_table_anim_in(int dir)
{
    (void)dir;
    uint32_t base = SCR_SLIDE_MS;

    /* Task count header */
    lv_obj_set_style_translate_y(g_tbl_cnt, 200, 0);
    lv_obj_set_style_translate_y(g_tbl_tsk, 200, 0);
    lv_obj_set_style_translate_y(g_idx_lbl, 200, 0);
    stacks_ty_anim(g_tbl_cnt, 200, 0, 450, base);
    stacks_ty_anim(g_tbl_tsk, 200, 0, 450, base + 40);
    stacks_ty_anim(g_idx_lbl, 200, 0, 420, base + 60);

    /* Active card: fade + subtle rise */
    lv_obj_set_style_opa(g_wrap[g_buf], LV_OPA_TRANSP, 0);
    lv_obj_set_style_translate_y(g_wrap[g_buf], 20, 0);
    stacks_opa_anim(g_wrap[g_buf], LV_OPA_TRANSP, LV_OPA_COVER, 300, base + 30);
    stacks_ty_anim(g_wrap[g_buf], 20, 0, 340, base + 30);
}
