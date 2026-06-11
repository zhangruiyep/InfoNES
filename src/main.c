#include "rtthread.h"
#include <rtdevice.h>
#include <string.h>
#include "bf0_hal.h"
#include "drv_io.h"
#include "dfs_file.h"
#include "drv_flash.h"
#include "drv_psram.h"
#include "littlevgl2rtt.h"
#include "lv_ex_data.h"

extern void nes_ui_init(void);

/* PSRAM bss section boundaries (from link.lds) */
extern unsigned int __RW_PSRAM_NON_RET_start__;
extern unsigned int __RW_PSRAM_NON_RET_end__;

#define FS_ROOT "root"

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

    /* init PSRAM before any NES buffer access */
    if (rt_psram_init() == 0) {
        rt_kprintf("[NES] PSRAM init OK\n");
        /* Zero-init PSRAM bss section */
        char *psram_bss_start = (char *)&__RW_PSRAM_NON_RET_start__;
        char *psram_bss_end   = (char *)&__RW_PSRAM_NON_RET_end__;
        size_t psram_bss_size = psram_bss_end - psram_bss_start;
        memset(psram_bss_start, 0, psram_bss_size);
        rt_kprintf("[NES] PSRAM bss zeroed: %d bytes\n", psram_bss_size);
    } else {
        rt_kprintf("[NES] PSRAM init FAILED\n");
    }

    /* mount FAT filesystem */
    register_mtd_device(FS_REGION_START_ADDR, FS_REGION_SIZE, FS_ROOT);
    if (dfs_mount(FS_ROOT, "/", "elm", 0, 0) == 0) {
        rt_kprintf("[NES] FAT fs mounted at /\n");
    } else {
        rt_kprintf("[NES] FAT fs not found, formatting...\n");
        if (dfs_mkfs("elm", FS_ROOT) == 0) {
            if (dfs_mount(FS_ROOT, "/", "elm", 0, 0) == 0) {
                rt_kprintf("[NES] FAT fs formatted and mounted\n");
            }
        }
    }

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
