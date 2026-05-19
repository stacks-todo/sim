#include "stacks_app.h"
#include <time.h>
#include <stdio.h>

static lv_obj_t      *g_screens[SCR_COUNT];
static stacks_screen_t g_cur = SCR_POMODORO;

/* ── Helpers ─────────────────────────────────────────────────── */

void stacks_clean_obj(lv_obj_t *o)
{
    lv_obj_set_style_bg_opa(o, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_outline_width(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t *stacks_make_circle(lv_obj_t *parent, int32_t d, lv_color_t color)
{
    lv_obj_t *o = lv_obj_create(parent);
    lv_obj_set_size(o, d, d);
    lv_obj_center(o);
    lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(o, color, 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_outline_width(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return o;
}

void stacks_create_bg(lv_obj_t *scr)
{
    lv_obj_set_style_bg_color(scr, CLR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    stacks_make_circle(scr, RING_OUTER_D, CLR_RING_OUTER);
    stacks_make_circle(scr, RING_GAP_D,   CLR_BG);
    stacks_make_circle(scr, RING_MID_D,   CLR_RING_MID);
    stacks_make_circle(scr, RING_INNER_D, CLR_INNER);
}

void stacks_get_datetime(char *buf, size_t len)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, len, "%b %d (%a)  %H:%M", t);
}

/* ── Swipe navigation ────────────────────────────────────────── */

#define SWIPE_THRESHOLD  60   /* px */

static lv_point_t g_drag_start;

static void swipe_nav(int32_t dx)
{
    if (dx < -SWIPE_THRESHOLD) {
        stacks_screen_t next = (g_cur + 1) % SCR_COUNT;
        lv_screen_load_anim(g_screens[next], LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
        g_cur = next;
    } else if (dx > SWIPE_THRESHOLD) {
        stacks_screen_t next = (g_cur + SCR_COUNT - 1) % SCR_COUNT;
        lv_screen_load_anim(g_screens[next], LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
        g_cur = next;
    }
}

/* Registered on the indev directly (not on objects) so it fires
 * regardless of which object is under the cursor, with no bubbling needed.
 * user_data = the indev pointer itself for lv_indev_get_point(). */
static void swipe_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_event_get_user_data(e);
    if (!indev) return;

    lv_point_t pt;
    lv_indev_get_point(indev, &pt);

    if (code == LV_EVENT_PRESSED) {
        g_drag_start = pt;
    } else if (code == LV_EVENT_RELEASED) {
        int32_t dx = pt.x - g_drag_start.x;
        int32_t dy = pt.y - g_drag_start.y;
        if (LV_ABS(dx) > LV_ABS(dy) * 2)
            swipe_nav(dx);
    }
}

/* ── Keyboard navigation ─────────────────────────────────────── */

static void key_nav_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_KEY) return;
    uint32_t key = lv_event_get_key(e);

    if (key == LV_KEY_RIGHT || key == LV_KEY_NEXT) {
        stacks_screen_t next = (g_cur + 1) % SCR_COUNT;
        lv_screen_load_anim(g_screens[next], LV_SCR_LOAD_ANIM_MOVE_LEFT, 250, 0, false);
        g_cur = next;
    } else if (key == LV_KEY_LEFT || key == LV_KEY_PREV) {
        stacks_screen_t next = (g_cur + SCR_COUNT - 1) % SCR_COUNT;
        lv_screen_load_anim(g_screens[next], LV_SCR_LOAD_ANIM_MOVE_RIGHT, 250, 0, false);
        g_cur = next;
    } else {
        /* forward UP/DOWN/ENTER/SPACE etc. to the active screen's handler */
        lv_obj_send_event(g_screens[g_cur], LV_EVENT_KEY, &key);
    }
}

/* ── Init ────────────────────────────────────────────────────── */

void stacks_app_init(void)
{
    g_screens[SCR_POMODORO] = screen_pomodoro_create();
    g_screens[SCR_CLOCK]    = screen_clock_create();
    g_screens[SCR_STACK]    = screen_stack_create();
    g_screens[SCR_TABLE]    = screen_table_create();

    lv_obj_t *nav = lv_obj_create(lv_layer_top());
    lv_obj_set_size(nav, 1, 1);
    stacks_clean_obj(nav);
    lv_obj_add_flag(nav, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(nav, key_nav_cb, LV_EVENT_KEY, NULL);
    lv_group_add_obj(lv_group_get_default(), nav);
    lv_group_focus_obj(nav);

    /* Swipe gesture: register directly on the pointer indev so it fires
     * regardless of which object is hit-tested (no bubbling required). */
    lv_indev_t *indev = lv_indev_get_next(NULL);
    while (indev) {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER) {
            lv_indev_add_event_cb(indev, swipe_cb, LV_EVENT_PRESSED,  indev);
            lv_indev_add_event_cb(indev, swipe_cb, LV_EVENT_RELEASED, indev);
            break;
        }
        indev = lv_indev_get_next(indev);
    }

    lv_screen_load(g_screens[SCR_POMODORO]);
}
