#include "stacks_app.h"
#include <math.h>
#include <stdio.h>

#define N_TASKS      12
#define CAROUSEL_R   280.0f   /* 3-D circle radius */
#define PERSP        420.0f   /* perspective distance */
#define CARD_W       260
#define CARD_H       68
#define ANIM_MS      300

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

/* ── carousel state ──────────────────────────────────────────────── */

typedef struct {
    lv_obj_t *card;
    float     theta;   /* current angle in 3-D circle (radians) */
} carousel_item_t;

static carousel_item_t g_items[N_TASKS];
static float           g_base  = 0.0f;   /* current base angle */
static lv_obj_t       *g_scr_ref = NULL;
static int             g_focused = 0;

static float lerp(float a, float b, float t) { return a + (b - a) * t; }

static void carousel_layout(void)
{
    float step = (float)(2.0 * M_PI) / N_TASKS;

    for (int i = 0; i < N_TASKS; i++) {
        float theta = g_base + i * step;
        g_items[i].theta = theta;

        float z    = cosf(theta);           /* -1 (back) .. +1 (front) */
        float x    = sinf(theta) * CAROUSEL_R * (PERSP / (PERSP + CAROUSEL_R));

        /* perspective scale: front=1.0, back=0.55 */
        float scale = lerp(0.55f, 1.0f, (z + 1.0f) * 0.5f);
        /* y offset: tilt the circle so front items sit lower */
        float y_off = -z * 30.0f;

        int32_t sw = (int32_t)(CARD_W * scale);
        int32_t sh = (int32_t)(CARD_H * scale);
        int32_t sx = DISP_CX - sw / 2 + (int32_t)x;
        int32_t sy = DISP_CY - sh / 2 + (int32_t)y_off;

        lv_obj_t *c = g_items[i].card;
        lv_obj_set_size(c, sw, sh);
        lv_obj_set_pos(c, sx, sy);

        /* opacity: front=255, back=60 */
        lv_opa_t opa = (lv_opa_t)lerp(60.0f, 255.0f, (z + 1.0f) * 0.5f);
        lv_obj_set_style_opa(c, opa, 0);

        /* z-order: higher z → move to front */
        if (z > 0) lv_obj_move_foreground(c);
        else       lv_obj_move_background(c);
    }
}

/* animated value callback used by lv_anim */
static void anim_exec_cb(void *var, int32_t val)
{
    /* val is angle * 1000 (fixed-point) */
    g_base = (float)val / 1000.0f;
    carousel_layout();
    (void)var;
}

static void carousel_rotate(int dir)
{
    float step = (float)(2.0 * M_PI) / N_TASKS;
    float target = g_base + dir * step;
    g_focused = (g_focused - dir + N_TASKS) % N_TASKS;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, anim_exec_cb);
    lv_anim_set_var(&a, NULL);
    lv_anim_set_values(&a,
                       (int32_t)(g_base  * 1000),
                       (int32_t)(target  * 1000));
    lv_anim_set_duration(&a, ANIM_MS);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
}

static void table_key_cb(lv_event_t *e)
{
    uint32_t key = lv_event_get_key(e);
    if      (key == LV_KEY_UP)   carousel_rotate(-1);
    else if (key == LV_KEY_DOWN) carousel_rotate( 1);
}

lv_obj_t *screen_table_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    stacks_create_bg(scr);
    g_scr_ref = scr;

    /* header */
    lv_obj_t *cnt = lv_label_create(scr);
    lv_label_set_text(cnt, "12");
    lv_obj_set_style_text_font(cnt, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(cnt, CLR_RED, 0);
    lv_obj_align(cnt, LV_ALIGN_CENTER, -20, -152);

    lv_obj_t *tsk = lv_label_create(scr);
    lv_label_set_text(tsk, "Tasks");
    lv_obj_set_style_text_font(tsk, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(tsk, CLR_GRAY, 0);
    lv_obj_align(tsk, LV_ALIGN_CENTER, 0, -112);

    char dt[40]; stacks_get_datetime(dt, sizeof(dt));
    lv_obj_t *dt_lbl = lv_label_create(scr);
    lv_label_set_text(dt_lbl, dt);
    lv_obj_set_style_text_font(dt_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(dt_lbl, CLR_GRAY, 0);
    lv_obj_align(dt_lbl, LV_ALIGN_CENTER, 0, -90);

    /* build cards (added to scr directly so we can z-sort freely) */
    for (int i = 0; i < N_TASKS; i++) {
        lv_obj_t *card = lv_obj_create(scr);
        lv_obj_set_size(card, CARD_W, CARD_H);
        lv_obj_set_style_radius(card, 12, 0);
        lv_obj_set_style_bg_color(card, CLR_CARD, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(card, 0, 0);
        lv_obj_set_style_outline_width(card, 0, 0);
        lv_obj_set_style_pad_left(card, 14, 0);
        lv_obj_set_style_pad_top(card, 10, 0);
        lv_obj_remove_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *title = lv_label_create(card);
        lv_label_set_text(title, TASKS[i].title);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_color(title, CLR_WHITE, 0);
        lv_label_set_long_mode(title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(title, 220);
        lv_obj_set_pos(title, 0, 0);

        lv_obj_t *dot = lv_obj_create(card);
        lv_obj_set_size(dot, 8, 8);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, dot_color(i), 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_set_pos(dot, 0, 32);

        lv_obj_t *due = lv_label_create(card);
        lv_label_set_text(due, TASKS[i].due);
        lv_obj_set_style_text_font(due, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(due, CLR_GRAY, 0);
        lv_obj_set_pos(due, 14, 28);

        g_items[i].card = card;
    }

    g_base    = 0.0f;
    g_focused = 0;
    carousel_layout();

    lv_obj_add_event_cb(scr, table_key_cb, LV_EVENT_KEY, NULL);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    return scr;
}
