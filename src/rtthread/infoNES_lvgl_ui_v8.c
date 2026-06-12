
#include "rtthread.h"
#include "lvgl.h"

#include "../InfoNES_System.h"

#define DBG_TAG "infoNES"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     240

#define NES_PAD_WIDTH   360
#define NES_PAD_HEIGHT  120
#define NES_BTN_SIZE    (NES_PAD_HEIGHT / 3)

typedef enum {
    NES_BTN_B,
    NES_BTN_A,
    NES_BTN_SELECT,
    NES_BTN_START,
    NES_BTN_UP,
    NES_BTN_DOWN,
    NES_BTN_LEFT,
    NES_BTN_RIGHT,
    NES_BTN_MAX,
} nes_btn_id_t;

unsigned short canvas_buffer[ NES_DISP_WIDTH * NES_DISP_HEIGHT ] __attribute__ ((aligned (4), section(".l2_non_ret_bss_nes")));
extern void start_application( void );
extern void close_application( void );

unsigned int dwKeyPad1;

typedef struct
{
    lv_obj_t *canvas;
    lv_obj_t *fps_label;
} app_nes_t;

static app_nes_t g_nes;
static lv_timer_t *g_refresh_timer = NULL;

/* =================================================================== */
/*  Button event handler (v8 signature)                                */
/* =================================================================== */
static void nes_btn_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    nes_btn_id_t btn_id = (nes_btn_id_t)(intptr_t)lv_obj_get_user_data(obj);

    if (code == LV_EVENT_PRESSED) {
        LOG_D("%s: btn %d press", __func__, btn_id);
        dwKeyPad1 |= (1 << btn_id);
    } else if (code == LV_EVENT_RELEASED) {
        LOG_D("%s: btn %d release", __func__, btn_id);
        dwKeyPad1 &= ~(1 << btn_id);
    }
}

/* =================================================================== */
/*  Pad (touch D-Pad + A/B/Select/Start)                               */
/* =================================================================== */
static lv_obj_t* nes_ui_btn_create(lv_obj_t *parent, nes_btn_id_t btn_id)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, NES_BTN_SIZE, NES_BTN_SIZE);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x808080), LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn, nes_btn_cb, LV_EVENT_PRESSED, (void *)(intptr_t)btn_id);
    lv_obj_add_event_cb(btn, nes_btn_cb, LV_EVENT_RELEASED, (void *)(intptr_t)btn_id);
    lv_obj_set_user_data(btn, (void *)(intptr_t)btn_id);
    return btn;
}

static void nes_ui_pad_init(lv_obj_t *parent)
{
    lv_obj_t *btn_up = nes_ui_btn_create(parent, NES_BTN_UP);
    lv_obj_align(btn_up, LV_ALIGN_LEFT_MID, NES_BTN_SIZE, -NES_BTN_SIZE);
    lv_obj_t *btn_down = nes_ui_btn_create(parent, NES_BTN_DOWN);
    lv_obj_align(btn_down, LV_ALIGN_LEFT_MID, NES_BTN_SIZE, NES_BTN_SIZE);
    lv_obj_t *btn_left = nes_ui_btn_create(parent, NES_BTN_LEFT);
    lv_obj_align(btn_left, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_t *btn_right = nes_ui_btn_create(parent, NES_BTN_RIGHT);
    lv_obj_align(btn_right, LV_ALIGN_LEFT_MID, NES_BTN_SIZE * 2, 0);

    lv_obj_t *btn_a = nes_ui_btn_create(parent, NES_BTN_A);
    lv_obj_align(btn_a, LV_ALIGN_RIGHT_MID, - NES_BTN_SIZE * 2, 0);
    lv_obj_t *btn_b = nes_ui_btn_create(parent, NES_BTN_B);
    lv_obj_align(btn_b, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t *btn_select = nes_ui_btn_create(parent, NES_BTN_SELECT);
    lv_obj_set_size(btn_select, NES_BTN_SIZE, NES_BTN_SIZE*2/3);
    lv_obj_align(btn_select, LV_ALIGN_BOTTOM_MID, -NES_BTN_SIZE/2, 0);
    lv_obj_t *btn_start = nes_ui_btn_create(parent, NES_BTN_START);
    lv_obj_set_size(btn_start, NES_BTN_SIZE, NES_BTN_SIZE*2/3);
    lv_obj_align(btn_start, LV_ALIGN_BOTTOM_MID, NES_BTN_SIZE/2, 0);
}

/* =================================================================== */
/*  Canvas refresh                                                      */
/* =================================================================== */
static bool nes_canvas_need_refresh = false;

static void nes_page_refresh(lv_timer_t *timer)
{
    (void)timer;
    if (nes_canvas_need_refresh && g_nes.canvas)
    {
        lv_obj_invalidate(g_nes.canvas);
        nes_canvas_need_refresh = false;
    }

    /* Update FPS display */
    if (g_nes.fps_label)
    {
        lv_label_set_text_fmt(g_nes.fps_label, "FPS: %d", g_nes_fps);
    }
}

void nes_canvas_refresh(void)
{
    nes_canvas_need_refresh = true;
}

/* =================================================================== */
/*  UI init (NES_SINGLE_APP mode)                                       */
/* =================================================================== */
static void nes_ui_obj_init(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);

    g_nes.canvas = lv_canvas_create(scr);
    lv_canvas_set_buffer(g_nes.canvas, canvas_buffer, NES_DISP_WIDTH, NES_DISP_HEIGHT, LV_IMG_CF_RGB565);
    lv_obj_align(g_nes.canvas, LV_ALIGN_TOP_MID, 0, 0);

    /* FPS label - below canvas, right side, black text */
    g_nes.fps_label = lv_label_create(scr);
    lv_label_set_text(g_nes.fps_label, "FPS: --");
    lv_obj_set_style_text_color(g_nes.fps_label, lv_color_black(), LV_STATE_DEFAULT);
    lv_obj_align_to(g_nes.fps_label, g_nes.canvas, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);

    lv_obj_t *pad = lv_obj_create(scr);
    lv_obj_set_size(pad, NES_PAD_WIDTH, NES_PAD_HEIGHT);
    lv_obj_align(pad, LV_ALIGN_BOTTOM_MID, 0, 0);

    nes_ui_pad_init(pad);
}

void nes_ui_init(void)
{
    rt_kprintf("[NES] nes_ui_obj_init start\n");
    nes_ui_obj_init();
    rt_kprintf("[NES] nes_ui_obj_init done\n");
    g_refresh_timer = lv_timer_create(nes_page_refresh, (1000/60), NULL);
    rt_kprintf("[NES] lv_timer_create done, calling start_application\n");
    start_application();
    rt_kprintf("[NES] start_application returned\n");
}
