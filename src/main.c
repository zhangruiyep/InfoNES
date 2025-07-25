#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "littlevgl2rtt.h"
#include "lv_ex_data.h"

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

    /* init littlevGL */
    ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK)
    {
        return ret;
    }
    lv_ex_data_pool_init();

    nes_ui_init();
    rt_kprintf("%s: start application\n", __func__);
    start_application();

    while (1)
    {
        nes_page_refresh();
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }
    return RT_EOK;

}
