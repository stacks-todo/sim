#include "stacks_app.h"

typedef struct { int16_t dx, dy, d; } bubble_t;

static const bubble_t BUBBLES[] = {
    /* large red    */ { -74,  52, 100 }, {  48,  64,  88 }, { -18, -28,  72 },
    /* medium orange */ {  60, -32,  60 }, { -52, -48,  56 }, {  20,  -8,  52 }, { -24,  52,  48 },
    /* small blue    */ { -90, -10,  32 }, {  80,  28,  30 }, { -60,  76,  28 },
    /* tiny          */ {  42, -68,  24 }, { -10,  92,  22 },
};

static lv_color_t bubble_color(int i)
{
    if (i < 3)  return CLR_RED;
    if (i < 7)  return CLR_ORANGE;
    if (i < 10) return CLR_BLUE;
    return CLR_ORANGE;
}

lv_obj_t *screen_stack_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    stacks_create_bg(scr);

    char dt[40]; stacks_get_datetime(dt, sizeof(dt));
    lv_obj_t *dt_lbl = lv_label_create(scr);
    lv_label_set_text(dt_lbl, dt);
    lv_obj_set_style_text_font(dt_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(dt_lbl, CLR_GRAY, 0);
    lv_obj_align(dt_lbl, LV_ALIGN_CENTER, 0, -148);

    int n = (int)(sizeof(BUBBLES) / sizeof(BUBBLES[0]));
    for (int i = 0; i < n; i++) {
        const bubble_t *b = &BUBBLES[i];
        lv_color_t col = bubble_color(i);

        lv_obj_t *o = lv_obj_create(scr);
        lv_obj_set_size(o, b->d, b->d);
        lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(o, col, 0);
        lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_grad_color(o, lv_color_darken(col, LV_OPA_20), 0);
        lv_obj_set_style_bg_grad_dir(o, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_main_stop(o, 0,   0);
        lv_obj_set_style_bg_grad_stop(o, 180, 0);
        lv_obj_set_style_border_width(o, 0, 0);
        lv_obj_set_style_outline_width(o, 0, 0);
        lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_pos(o,
                       DISP_CX + b->dx - b->d / 2,
                       DISP_CY + b->dy - b->d / 2);
    }

    /* Task count on largest bubble */
    lv_obj_t *cnt = lv_label_create(scr);
    lv_label_set_text(cnt, "12");
    lv_obj_set_style_text_font(cnt, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(cnt, CLR_WHITE, 0);
    lv_obj_set_pos(cnt,
                   DISP_CX + BUBBLES[0].dx - 18,
                   DISP_CY + BUBBLES[0].dy - 22);

    lv_obj_t *tsk = lv_label_create(scr);
    lv_label_set_text(tsk, "Tasks");
    lv_obj_set_style_text_font(tsk, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(tsk, CLR_GRAY, 0);
    lv_obj_align(tsk, LV_ALIGN_CENTER, 0, 168);

    return scr;
}
