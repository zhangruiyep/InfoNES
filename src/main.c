#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#if NES_USING_LVGL
#include "littlevgl2rtt.h"
#include "lv_ex_data.h"
#endif

extern void start_application( void );
extern void close_application( void );
extern void nes_ui_init(void);
extern void nes_page_refresh(void);

/**
  * @brief  Main program
  * @param  None
  * @retval 0 if success, otherwise failure number
  */
int main(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t ms;

#if NES_USING_LVGL
    /* init littlevGL */
    ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK)
    {
        return ret;
    }
    lv_ex_data_pool_init();
#else

#endif

    nes_ui_init();
    rt_kprintf("%s: start application\n", __func__);
    start_application();

    while (1)
    {
#if NES_USING_LVGL
        nes_page_refresh();
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
#else
        rt_thread_mdelay(1000);
#endif
    }
    return RT_EOK;

}
