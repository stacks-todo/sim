/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t * hal_init(int32_t w, int32_t h);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char **argv)
{
  lv_init();

  hal_init(480, 320);

  if (argc <= 1) {
    lv_demo_widgets();
  } else {
    if (!lv_demos_create(&argv[1], argc - 1)) {
      lv_demos_show_help();
      goto demo_end;
    }
  }

  while(1) {
    uint32_t time_until_next = lv_timer_handler();

    if (time_until_next == LV_NO_TIMER_READY) {
      time_until_next = LV_DEF_REFR_PERIOD;
    }

    lv_delay_ms(time_until_next);
  }

demo_end:
  lv_deinit();
  return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_display_t * hal_init(int32_t w, int32_t h)
{
  lv_group_set_default(lv_group_create());

  lv_display_t * disp = lv_sdl_window_create(w, h);

  lv_indev_t * mouse = lv_sdl_mouse_create();
  lv_indev_set_group(mouse, lv_group_get_default());
  lv_indev_set_display(mouse, disp);
  lv_display_set_default(disp);

  LV_IMAGE_DECLARE(mouse_cursor_icon);
  lv_obj_t * cursor_obj;
  cursor_obj = lv_image_create(lv_screen_active());
  lv_image_set_src(cursor_obj, &mouse_cursor_icon);
  lv_indev_set_cursor(mouse, cursor_obj);

  lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
  lv_indev_set_display(mousewheel, disp);

  lv_indev_t * keyboard = lv_sdl_keyboard_create();
  lv_indev_set_display(keyboard, disp);
  lv_indev_set_group(keyboard, lv_group_get_default());

  return disp;
}
