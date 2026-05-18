#include "lvgl/lvgl.h"
#include "stacks_app.h"

static lv_display_t *hal_init(int32_t w, int32_t h);

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    lv_init();
    hal_init(DISP_W, DISP_H);
    stacks_app_init();

    while (1) {
        uint32_t t = lv_timer_handler();
        if (t == LV_NO_TIMER_READY) t = LV_DEF_REFR_PERIOD;
        lv_delay_ms(t);
    }

    lv_deinit();
    return 0;
}

static lv_display_t *hal_init(int32_t w, int32_t h)
{
    lv_group_set_default(lv_group_create());

    lv_display_t *disp = lv_sdl_window_create(w, h);

    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
    lv_indev_set_display(mouse, disp);
    lv_display_set_default(disp);

    lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_display(mousewheel, disp);

    lv_indev_t *keyboard = lv_sdl_keyboard_create();
    lv_indev_set_display(keyboard, disp);
    lv_indev_set_group(keyboard, lv_group_get_default());

    return disp;
}
