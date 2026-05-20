#include "stacks_app.h"
#include "anim_utils.h"
#include <math.h>
#include <stdlib.h>

#define N_BUBBLES   12
#define WALL_R      154.0f   /* inner circle radius = 328/2 */
#define GRAVITY     0.18f    /* downward acceleration per frame */
#define DAMPING     0.985f
#define DT          0.016f   /* ~60fps */

typedef struct {
    float x, y;
    float vx, vy;
    float r;
    lv_color_t color;
    lv_obj_t  *obj;
} phys_bubble_t;

static phys_bubble_t g_bub[N_BUBBLES];
static lv_obj_t     *g_stack_dt_lbl;

static const struct { int16_t dx, dy; uint8_t d; } INIT[] = {
    { -74,  52, 100 }, {  48,  64,  88 }, { -18, -28,  72 },
    {  60, -32,  60 }, { -52, -48,  56 }, {  20,  -8,  52 }, { -24,  52,  48 },
    { -90, -10,  32 }, {  80,  28,  30 }, { -60,  76,  28 },
    {  42, -68,  24 }, { -10,  92,  22 },
};

static lv_color_t bubble_color(int i)
{
    if (i < 3)  return CLR_RED;
    if (i < 7)  return CLR_ORANGE;
    if (i < 10) return CLR_BLUE;
    return CLR_ORANGE;
}

static void phys_step(void)
{
    /* integrate */
    for (int i = 0; i < N_BUBBLES; i++) {
        phys_bubble_t *b = &g_bub[i];
        /* gravity */
        b->vy += GRAVITY;
        b->vx *= DAMPING;
        b->vy *= DAMPING;
        b->x  += b->vx;
        b->y  += b->vy;
    }

    /* circle-circle collisions */
    for (int i = 0; i < N_BUBBLES - 1; i++) {
        for (int j = i + 1; j < N_BUBBLES; j++) {
            phys_bubble_t *a = &g_bub[i];
            phys_bubble_t *b = &g_bub[j];
            float dx = b->x - a->x;
            float dy = b->y - a->y;
            float dist2 = dx*dx + dy*dy;
            float minD  = a->r + b->r;
            if (dist2 < minD * minD && dist2 > 0.0001f) {
                float dist  = sqrtf(dist2);
                float nx    = dx / dist;
                float ny    = dy / dist;
                float overlap = (minD - dist) * 0.5f;
                a->x -= nx * overlap;
                a->y -= ny * overlap;
                b->x += nx * overlap;
                b->y += ny * overlap;
                /* velocity exchange along normal */
                float dvx = b->vx - a->vx;
                float dvy = b->vy - a->vy;
                float dot  = dvx * nx + dvy * ny;
                if (dot < 0) {
                    float impulse = dot * 0.9f; /* restitution */
                    a->vx += impulse * nx;
                    a->vy += impulse * ny;
                    b->vx -= impulse * nx;
                    b->vy -= impulse * ny;
                }
            }
        }
    }

    /* circular wall collision */
    for (int i = 0; i < N_BUBBLES; i++) {
        phys_bubble_t *b = &g_bub[i];
        float dist = sqrtf(b->x * b->x + b->y * b->y);
        float limit = WALL_R - b->r;
        if (dist > limit && dist > 0.001f) {
            float nx = b->x / dist;
            float ny = b->y / dist;
            b->x = nx * limit;
            b->y = ny * limit;
            float dot = b->vx * nx + b->vy * ny;
            if (dot > 0) {
                b->vx -= 1.8f * dot * nx;
                b->vy -= 1.8f * dot * ny;
            }
        }
    }

    /* update LVGL positions */
    for (int i = 0; i < N_BUBBLES; i++) {
        phys_bubble_t *b = &g_bub[i];
        lv_obj_set_pos(b->obj,
                       (int32_t)(DISP_CX + b->x - b->r),
                       (int32_t)(DISP_CY + b->y - b->r));
    }
}

static void phys_timer_cb(lv_timer_t *t)
{
    (void)t;
    phys_step();
}

lv_obj_t *screen_stack_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    stacks_create_bg(scr);

    char dt[40]; stacks_get_datetime(dt, sizeof(dt));
    g_stack_dt_lbl = lv_label_create(scr);
    lv_label_set_text(g_stack_dt_lbl, dt);
    lv_obj_set_style_text_font(g_stack_dt_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_stack_dt_lbl, CLR_GRAY, 0);
    lv_obj_align(g_stack_dt_lbl, LV_ALIGN_CENTER, 0, -148);

    /* init physics bubbles */
    for (int i = 0; i < N_BUBBLES; i++) {
        phys_bubble_t *b = &g_bub[i];
        b->x  = INIT[i].dx;
        b->y  = INIT[i].dy;
        b->r  = INIT[i].d * 0.5f;
        b->vx = ((float)(rand() % 200) - 100) * 0.01f;
        b->vy = ((float)(rand() % 200) - 100) * 0.01f;
        b->color = bubble_color(i);

        int32_t d = INIT[i].d;
        lv_obj_t *o = lv_obj_create(scr);
        lv_obj_set_size(o, d, d);
        lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(o, b->color, 0);
        lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_grad_color(o, lv_color_darken(b->color, LV_OPA_20), 0);
        lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_main_stop(o, 0,   0);
        lv_obj_set_style_bg_grad_stop(o, 180, 0);
        lv_obj_set_style_border_width(o, 0, 0);
        lv_obj_set_style_outline_width(o, 0, 0);
        lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_pos(o,
                       DISP_CX + INIT[i].dx - d / 2,
                       DISP_CY + INIT[i].dy - d / 2);
        b->obj = o;
    }

    /* task count on largest bubble — repositioned each frame via physics */
    lv_obj_t *cnt = lv_label_create(scr);
    lv_label_set_text(cnt, "12");
    lv_obj_set_style_text_font(cnt, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(cnt, CLR_WHITE, 0);
    lv_obj_set_pos(cnt,
                   DISP_CX + INIT[0].dx - 18,
                   DISP_CY + INIT[0].dy - 22);

    lv_obj_t *tsk = lv_label_create(scr);
    lv_label_set_text(tsk, "Tasks");
    lv_obj_set_style_text_font(tsk, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(tsk, CLR_GRAY, 0);
    lv_obj_align(tsk, LV_ALIGN_CENTER, 0, 168);

    lv_timer_create(phys_timer_cb, 16, NULL);

    return scr;
}

/* Entrance animation — mirrors canvasWrap translate from firmware.
 *
 * Date label slides in from the arriving side.
 * Bubbles fade in with a stagger so they "appear" into the container
 * (approximates the canvasWrap width/scale expand from the firmware).
 */
void screen_stack_anim_in(int dir)
{
    int32_t off = (int32_t)(dir * 200);

    lv_obj_set_style_translate_x(g_stack_dt_lbl, off, 0);
    stacks_tx_anim(g_stack_dt_lbl, off, 0, 360, 0);

    for (int i = 0; i < N_BUBBLES; i++) {
        lv_obj_set_style_opa(g_bub[i].obj, LV_OPA_TRANSP, 0);
        stacks_opa_anim(g_bub[i].obj, LV_OPA_TRANSP, LV_OPA_COVER,
                        360, (uint32_t)(i * 18));
    }
}
