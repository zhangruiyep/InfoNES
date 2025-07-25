
#include "rtthread.h"
#include "littlevgl2rtt.h"
#include "mem_section.h"

#define LOG_D rt_kprintf

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     240

unsigned short canvas_buffer[ NES_DISP_WIDTH * NES_DISP_HEIGHT ] __attribute__ ((aligned (4)));
extern void start_application( void );
extern void close_application( void );

unsigned int dwKeyPad1;

typedef struct
{
    lv_obj_t              *bg_page;
    rt_list_t             *list;
    //lv_task_t             *refresh_task;
    lv_obj_t              *canvas;
    lv_obj_t              *pad;
} app_nes_t;

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

static app_nes_t *p_nes = NULL;


static void nes_btn_cb(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    nes_btn_id_t btn_id = (nes_btn_id_t)lv_event_get_user_data(event);

    if (code == LV_EVENT_PRESSED) {
        LOG_D("%s: btn %d press", __func__, btn_id);
        dwKeyPad1 |= (1 << btn_id);
    } else if (code == LV_EVENT_RELEASED) {
        LOG_D("%s: btn %d release", __func__, btn_id);
        dwKeyPad1 &= ~(1 << btn_id);
    }
}
static lv_obj_t* nes_ui_btn_create(lv_obj_t *parent, nes_btn_id_t btn_id)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, NES_BTN_SIZE, NES_BTN_SIZE);
    lv_obj_set_style_bg_color(btn, LV_COLOR_GRAY, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn, nes_btn_cb, LV_EVENT_ALL, (void *)btn_id);
    return btn;
}

static void nes_ui_pad_init(lv_obj_t *parent)
{
    // + buttons
    lv_obj_t *btn_up = nes_ui_btn_create(parent, NES_BTN_UP);
    lv_obj_align(btn_up, LV_ALIGN_LEFT_MID, NES_BTN_SIZE, -NES_BTN_SIZE);
    lv_obj_t *btn_down = nes_ui_btn_create(parent, NES_BTN_DOWN);
    lv_obj_align(btn_down, LV_ALIGN_LEFT_MID, NES_BTN_SIZE, NES_BTN_SIZE);
    lv_obj_t *btn_left = nes_ui_btn_create(parent, NES_BTN_LEFT);
    lv_obj_align(btn_left, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_t *btn_right = nes_ui_btn_create(parent, NES_BTN_RIGHT);
    lv_obj_align(btn_right, LV_ALIGN_LEFT_MID, NES_BTN_SIZE * 2, 0);
    // ab buttons
    lv_obj_t *btn_a = nes_ui_btn_create(parent, NES_BTN_A);
    lv_obj_align(btn_a, LV_ALIGN_RIGHT_MID, - NES_BTN_SIZE * 2, 0);
    lv_obj_t *btn_b = nes_ui_btn_create(parent, NES_BTN_B);
    lv_obj_align(btn_b, LV_ALIGN_RIGHT_MID, 0, 0);
    // select/start buttons
    lv_obj_t *btn_select = nes_ui_btn_create(parent, NES_BTN_SELECT);
    lv_obj_set_size(btn_select, NES_BTN_SIZE, NES_BTN_SIZE*2/3);
    lv_obj_align(btn_select, LV_ALIGN_BOTTOM_MID, -NES_BTN_SIZE/2, 0);
    lv_obj_t *btn_start = nes_ui_btn_create(parent, NES_BTN_START);
    lv_obj_set_size(btn_start, NES_BTN_SIZE, NES_BTN_SIZE*2/3);
    lv_obj_align(btn_start, LV_ALIGN_BOTTOM_MID, NES_BTN_SIZE/2, 0);
}

void nes_ui_obj_init(lv_obj_t *parent)
{
    // 创建画布
    p_nes->canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(p_nes->canvas, canvas_buffer, NES_DISP_WIDTH, NES_DISP_HEIGHT, LV_IMG_CF_RGB565);
    lv_obj_align(p_nes->canvas, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *pad = lv_obj_create(parent);
    lv_obj_set_size(pad, NES_PAD_WIDTH, NES_PAD_HEIGHT);
    lv_obj_align(pad, LV_ALIGN_BOTTOM_MID, 0, -20);

    nes_ui_pad_init(pad);
}

/* call lv functions in lv task */
static bool nes_canvas_need_refresh = false;
//void nes_page_refresh(lv_task_t *task)
void nes_page_refresh(void)
{
    if (nes_canvas_need_refresh)
    {
        lv_obj_invalidate (p_nes->canvas);
        nes_canvas_need_refresh = false;
    }
}

void nes_canvas_refresh(void)
{
    nes_canvas_need_refresh = true;
}


void nes_ui_init(void)
{
    p_nes = (app_nes_t *)rt_malloc(sizeof(app_nes_t));
    memset(p_nes, 0, sizeof(app_nes_t));
    lv_obj_t *bg_page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(bg_page, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    p_nes->bg_page = bg_page;
    nes_ui_obj_init(p_nes->bg_page);
}
#if 0
static void on_start(void)
{
    p_nes = (app_nes_t *)APP_GET_PAGE_MEM_PTR;
    lv_obj_t *cont = lv_basecont_parent_create(lv_scr_act());

    lv_obj_t *title = lvsf_title_create(cont, NULL);
    lvsf_title_set_text(title, app_get_str(key_nes, "Nes"));
    lvsf_title_bottom_set_icons(title, APP_GET_IMG_FROM_APP(symbol, img_title_backspace));
    lvsf_title_set_visible_item(title, LVSF_TITLE_TITLE | LVSF_TITLE_TIME | LVSF_TITLE_BACK_BTN | LVSF_TITLE_BACK_BTN_ICONS);

    lv_obj_t *bg_page = lvsf_page_create(cont, NULL);
    lvsf_page_set_scrollbar_mode(bg_page, LV_SCROLLBAR_MODE_OFF);
    lvsf_page_set_size(bg_page, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL) - lv_obj_get_height(title) - lv_obj_get_y(title));
    lvsf_page_set_align(bg_page, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_ext_set_local_bg(bg_page, LV_COLOR_WHITE, LV_OPA_COVER);
    lvsf_page_set_status(bg_page, PAGE_STATE_STARTED);
    p_nes->bg_page = bg_page;
    nes_ui_obj_init(p_nes->bg_page);
    lvsf_page_set_defult_pos(p_nes->bg_page);

    //keypad_handler_register(nes_keypad_handler_cb);
    LOG_D("%s: start application", __func__);
    start_application();
}

static void on_resume(void)
{
    app_disable_screen_lock_time();
    RT_ASSERT(p_nes);
    if (PAGE_STATE_STARTED != lvsf_page_get_status(p_nes->bg_page))
    {
        lv_page_clean(p_nes->bg_page);
        nes_ui_obj_init(p_nes->bg_page);
        lvsf_page_set_defult_pos(p_nes->bg_page);
    }
    lvsf_page_set_drag_dir(p_nes->bg_page, DRAG_DIR_VER);
    lvsf_page_set_focus_enable(p_nes->bg_page, true);
    lvsf_page_focus_entry(p_nes->bg_page);
    lvsf_page_set_scrlbar_enable(p_nes->bg_page, true);
    lvsf_page_set_status(p_nes->bg_page, PAGE_STATE_RESUMED);
    /* refresh task */
    if (NULL == p_nes->refresh_task)
    {
        p_nes->refresh_task = lv_task_create(nes_page_refresh, (1000/60), LV_TASK_PRIO_LOW, (void *)0);
    }
    nes_page_refresh(NULL);
}

static void on_pause(void)
{
    RT_ASSERT(p_nes);
    lvsf_page_set_focus_disable(p_nes->bg_page);
    lvsf_page_set_scrlbar_enable(p_nes->bg_page, false);
    lvsf_page_set_status(p_nes->bg_page, PAGE_STATE_PAUSED);
    app_enable_screen_lock_time();
    if (p_nes->refresh_task)
    {
        lv_task_del(p_nes->refresh_task);
        p_nes->refresh_task = NULL;
    }
}

static void on_stop(void)
{
    LOG_D("%s: stop application", __func__);
    close_application();
    //keypad_handler_register(NULL);
    RT_ASSERT(p_nes);
    p_nes = NULL;
}

APP_MSG_HANDLER_REGISTER(on_start, on_resume, on_pause, on_stop);
APPLICATION_REGISTER(app_get_strid(key_nes, "Nes"), img_menu_dice,
                     "Nes", sizeof(app_nes_t));
#endif
