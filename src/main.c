#include "rtthread.h"
#include "bf0_hal.h"
#include "drv_io.h"
#include "littlevgl2rtt.h"
#include "lv_ex_data.h"

extern void nes_ui_init(void);

/**
  * @brief  Main program
  * @param  None
  * @retval 0 if success, otherwise failure number
  */
int main(void)
{
    rt_err_t ret = RT_EOK;
    rt_uint32_t ms;

    rt_kprintf("[NES] main() start\n");

    /* init littlevGL */
    ret = littlevgl2rtt_init("lcd");
    if (ret != RT_EOK)
    {
        rt_kprintf("[NES] littlevgl2rtt_init failed: %d\n", ret);
        return ret;
    }
    rt_kprintf("[NES] LVGL init done\n");
    lv_ex_data_pool_init();
    rt_kprintf("[NES] lv_ex_data_pool_init done\n");

    rt_kprintf("[NES] calling nes_ui_init()...\n");
    nes_ui_init();
    rt_kprintf("[NES] nes_ui_init() returned\n");

    while (1)
    {
        ms = lv_task_handler();
        rt_thread_mdelay(ms);
    }
    return RT_EOK;

}
