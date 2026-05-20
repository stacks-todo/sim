#pragma once
#include "lvgl/lvgl.h"

/* ── Display ─────────────────────────────────────────────────── */
#define DISP_W   480
#define DISP_H   480
#define DISP_CX  (DISP_W / 2)
#define DISP_CY  (DISP_H / 2)

/* ── Colors ──────────────────────────────────────────────────── */
#define CLR_BG          lv_color_hex(0x0D0D0D)
#define CLR_RING_OUTER  lv_color_hex(0x2B2B2B)
#define CLR_RING_MID    lv_color_hex(0x1E1E1E)
#define CLR_INNER       lv_color_hex(0x121212)
#define CLR_RED         lv_color_hex(0xE53935)
#define CLR_ORANGE      lv_color_hex(0xFF8C00)
#define CLR_BLUE        lv_color_hex(0x29B6F6)
#define CLR_WHITE       lv_color_hex(0xFFFFFF)
#define CLR_GRAY        lv_color_hex(0x757575)
#define CLR_GRAY_LIGHT  lv_color_hex(0xAAAAAA)
#define CLR_BTN         lv_color_hex(0x3D3D3D)
#define CLR_CARD        lv_color_hex(0x282828)

/* ── Ring diameters ───────────────────────────────────────────── */
#define RING_OUTER_D   460
#define RING_GAP_D     408
#define RING_MID_D     388
#define RING_INNER_D   328

/* ── Screen IDs ───────────────────────────────────────────────── */
typedef enum {
    SCR_POMODORO = 0,
    SCR_CLOCK,
    SCR_STACK,
    SCR_TABLE,
    SCR_COUNT
} stacks_screen_t;

/* ── Public API ───────────────────────────────────────────────── */
void       stacks_app_init(void);
void       stacks_create_bg(lv_obj_t *scr);
void       stacks_get_datetime(char *buf, size_t len);
void       stacks_clean_obj(lv_obj_t *o);
lv_obj_t  *stacks_make_circle(lv_obj_t *parent, int32_t d, lv_color_t color);

lv_obj_t  *screen_pomodoro_create(void);
lv_obj_t  *screen_clock_create(void);
lv_obj_t  *screen_stack_create(void);
lv_obj_t  *screen_table_create(void);

/* Duration of the screen-slide used by stacks_go_to().
 * anim_in functions add this as a base delay so per-element animations
 * kick in exactly when the new screen has finished landing. */
#define SCR_SLIDE_MS 280

/* Entrance animations — called just before lv_screen_load_anim.
 * dir: +1 = screen slides in from the right (→ navigation)
 *      -1 = screen slides in from the left  (← navigation)  */
void screen_pomodoro_anim_in(int dir);
void screen_clock_anim_in(int dir);
void screen_stack_anim_in(int dir);
void screen_table_anim_in(int dir);
