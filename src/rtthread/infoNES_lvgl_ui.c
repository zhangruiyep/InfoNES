
#include "rtthread.h"
#include "global.h"
#define _MODULE_NAME_ "infoNES"
#include "app_module.h"

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     240

//extern WORD WorkFrame[ NES_DISP_WIDTH * NES_DISP_HEIGHT ];
unsigned short canvas_buffer[ NES_DISP_WIDTH * NES_DISP_HEIGHT ];
extern void start_application( void );
extern void close_application( void );

typedef struct
{
    lv_obj_t              *bg_page;
    rt_list_t             *list;
    lv_task_t             *refresh_task;
    lv_obj_t              *canvas;
} app_nes_t;


static app_nes_t *p_nes = NULL;

void nes_ui_obj_init(lv_obj_t *parent)
{
    // 创建画布
    p_nes->canvas = lv_canvas_create(parent, NULL);
    lv_canvas_set_buffer(p_nes->canvas, canvas_buffer, NES_DISP_WIDTH, NES_DISP_HEIGHT, LV_IMG_CF_RGB565);
    lv_obj_align(p_nes->canvas, parent, LV_ALIGN_IN_TOP_MID, 0, 0);
}

void nes_page_refresh(lv_task_t *task)
{

}

void nes_canvas_refresh(void)
{
    lv_obj_invalidate (p_nes->canvas);
}

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
        p_nes->refresh_task = lv_task_create(nes_page_refresh, CLOCK_REFRESH_PERIOD, LV_TASK_PRIO_LOW, (void *)0);
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
