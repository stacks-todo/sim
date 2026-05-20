#pragma once
#include "lvgl/lvgl.h"

/* Custom cubic-bezier path callbacks for lv_anim_t.
 *
 * EASE_OUT : cubic-bezier(0.086, 0.875, 0.304, 1.0)  — fast start, gentle settle
 * EASE_IN  : cubic-bezier(0.742, 0.000, 0.875, 0.322) — slow start, fast finish
 *
 * Curves ported from src/lib/easings.ts in the STACKS firmware repo.
 */
int32_t stacks_ease_out(const lv_anim_t *a);
int32_t stacks_ease_in(const lv_anim_t *a);

/* Convenience wrappers — start an lv_anim on a single style property.
 * All use EASE_OUT unless noted. `delay` is in ms. */
void stacks_tx_anim(lv_obj_t *obj, int32_t from, int32_t to,
                    uint32_t dur_ms, uint32_t delay_ms);
void stacks_ty_anim(lv_obj_t *obj, int32_t from, int32_t to,
                    uint32_t dur_ms, uint32_t delay_ms);
void stacks_opa_anim(lv_obj_t *obj, int32_t from, int32_t to,
                     uint32_t dur_ms, uint32_t delay_ms);
