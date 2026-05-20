#include "anim_utils.h"

/* ── Cubic Bézier solver ─────────────────────────────────────────
 *
 * A cubic Bézier in [0,1]×[0,1] with endpoints (0,0) and (1,1):
 *   x(t) = 3(1-t)²t·x1 + 3(1-t)t²·x2 + t³
 *   y(t) = 3(1-t)²t·y1 + 3(1-t)t²·y2 + t³
 *
 * Given a normalised time u ∈ [0,1] (= act_time / duration) we find t
 * with Newton–Raphson so that x(t) = u, then return y(t).
 */

static float bx(float t, float x1, float x2)
{
    float u = 1.0f - t;
    return 3.0f*u*u*t*x1 + 3.0f*u*t*t*x2 + t*t*t;
}

static float by(float t, float y1, float y2)
{
    float u = 1.0f - t;
    return 3.0f*u*u*t*y1 + 3.0f*u*t*t*y2 + t*t*t;
}

static float dbx(float t, float x1, float x2)
{
    float u = 1.0f - t;
    return 3.0f*u*u*x1 + 6.0f*u*t*(x2 - x1) + 3.0f*t*t*(1.0f - x2);
}

static float cubic_bezier(float u, float x1, float y1, float x2, float y2)
{
    if (u <= 0.0f) return 0.0f;
    if (u >= 1.0f) return 1.0f;

    float t = u;
    for (int i = 0; i < 10; i++) {
        float d = dbx(t, x1, x2);
        if (d < 1e-6f && d > -1e-6f) break;
        t -= (bx(t, x1, x2) - u) / d;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
    }
    return by(t, y1, y2);
}

/* ── Path callbacks ─────────────────────────────────────────────── */

/* EASE_OUT: M0,0 C0.086,0.875 0.304,1 1,1 */
int32_t stacks_ease_out(const lv_anim_t *a)
{
    if (a->act_time <= 0)            return a->start_value;
    if (a->act_time >= a->duration)  return a->end_value;
    float u = (float)a->act_time / (float)a->duration;
    float p = cubic_bezier(u, 0.086f, 0.875f, 0.304f, 1.0f);
    return a->start_value + (int32_t)((float)(a->end_value - a->start_value) * p + 0.5f);
}

/* EASE_IN: M0,0 C0.742,0 0.875,0.322 1,1 */
int32_t stacks_ease_in(const lv_anim_t *a)
{
    if (a->act_time <= 0)            return a->start_value;
    if (a->act_time >= a->duration)  return a->end_value;
    float u = (float)a->act_time / (float)a->duration;
    float p = cubic_bezier(u, 0.742f, 0.0f, 0.875f, 0.322f);
    return a->start_value + (int32_t)((float)(a->end_value - a->start_value) * p + 0.5f);
}

/* ── Style setters (match lv_anim_exec_xcb_t signature) ────────── */

static void _set_tx(void *obj, int32_t v)
{
    lv_obj_set_style_translate_x((lv_obj_t *)obj, v, 0);
}

static void _set_ty(void *obj, int32_t v)
{
    lv_obj_set_style_translate_y((lv_obj_t *)obj, v, 0);
}

static void _set_opa(void *obj, int32_t v)
{
    lv_opa_t opa = (lv_opa_t)(v < 0 ? 0 : v > 255 ? 255 : v);
    lv_obj_set_style_opa((lv_obj_t *)obj, opa, 0);
}

/* ── Internal launcher ──────────────────────────────────────────── */

static void _launch(lv_obj_t *obj, lv_anim_exec_xcb_t exec_cb,
                    int32_t from, int32_t to,
                    uint32_t dur_ms, uint32_t delay_ms,
                    lv_anim_path_cb_t path_cb)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, exec_cb);
    lv_anim_set_values(&a, from, to);
    lv_anim_set_duration(&a, (int32_t)dur_ms);
    lv_anim_set_delay(&a, (int32_t)delay_ms);
    lv_anim_set_path_cb(&a, path_cb);
    lv_anim_start(&a);
}

/* ── Public helpers ─────────────────────────────────────────────── */

void stacks_tx_anim(lv_obj_t *obj, int32_t from, int32_t to,
                    uint32_t dur_ms, uint32_t delay_ms)
{
    _launch(obj, _set_tx, from, to, dur_ms, delay_ms, stacks_ease_out);
}

void stacks_ty_anim(lv_obj_t *obj, int32_t from, int32_t to,
                    uint32_t dur_ms, uint32_t delay_ms)
{
    _launch(obj, _set_ty, from, to, dur_ms, delay_ms, stacks_ease_out);
}

void stacks_opa_anim(lv_obj_t *obj, int32_t from, int32_t to,
                     uint32_t dur_ms, uint32_t delay_ms)
{
    _launch(obj, _set_opa, from, to, dur_ms, delay_ms, stacks_ease_out);
}

/* ── EASE_IN (exit/out) variants ────────────────────────────────── */

void stacks_ty_anim_out(lv_obj_t *obj, int32_t from, int32_t to,
                        uint32_t dur_ms, uint32_t delay_ms)
{
    _launch(obj, _set_ty, from, to, dur_ms, delay_ms, stacks_ease_in);
}

void stacks_opa_anim_out(lv_obj_t *obj, int32_t from, int32_t to,
                         uint32_t dur_ms, uint32_t delay_ms)
{
    _launch(obj, _set_opa, from, to, dur_ms, delay_ms, stacks_ease_in);
}
