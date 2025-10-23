
#include "rtthread.h"
#include "mem_section.h"

#define LOG_D rt_kprintf

/* NES display size */
#define NES_DISP_WIDTH      256
#define NES_DISP_HEIGHT     240

unsigned short lcd_buffer[ NES_DISP_WIDTH * NES_DISP_HEIGHT ] __attribute__ ((aligned (4)));
extern void start_application( void );
extern void close_application( void );

unsigned int dwKeyPad1 = 0;

static rt_device_t g_lcd_device = RT_NULL;
static struct rt_device_graphic_info lcd_info;

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

static void set_brightness(rt_device_t lcd_device)
{
    //rt_err_t err = rt_device_open(lcd_device, RT_DEVICE_OFLAG_RDWR);
    //if ((RT_EOK == err) || (-RT_EBUSY == err))
    {
        uint8_t brightness = 100;
        rt_device_control(lcd_device, RTGRAPHIC_CTRL_SET_BRIGHTNESS, &brightness);

        //if (RT_EOK == err) rt_device_close(lcd_device);
    }
}

void nes_ui_init(void)
{
    /* use lcd device api instead of littlevGL */
    rt_err_t err;
    g_lcd_device = rt_device_find("lcd");
    if (!g_lcd_device)
    {
        rt_kprintf("Can't find lcd\n");
        return;
    }

    err = rt_device_open(g_lcd_device, RT_DEVICE_OFLAG_RDWR);
    if (RT_EOK != err)
    {
        rt_kprintf("lcd open err %d\n", err);
        return;
    }

    if (rt_device_control(g_lcd_device, RTGRAPHIC_CTRL_GET_INFO, &lcd_info) == RT_EOK)
    {
        rt_kprintf("Lcd info w:%d, h%d, bits_per_pixel %d\r\n", lcd_info.width, lcd_info.height, lcd_info.bits_per_pixel);
    }

    uint16_t cf;
    if (16 == lcd_info.bits_per_pixel)
        cf = RTGRAPHIC_PIXEL_FORMAT_RGB565;
    else if (24 == lcd_info.bits_per_pixel)
        cf = RTGRAPHIC_PIXEL_FORMAT_RGB888;
    else
        RT_ASSERT(0);

    rt_device_control(g_lcd_device, RTGRAPHIC_CTRL_SET_BUF_FORMAT, &cf);

    int32_t dx = (LCD_HOR_RES_MAX - NES_DISP_WIDTH) / 2;
    int32_t dy = (LCD_VER_RES_MAX - NES_DISP_HEIGHT) / 2;
    rt_graphix_ops(g_lcd_device)->set_window(dx, dy, dx + NES_DISP_WIDTH - 1, dy + NES_DISP_HEIGHT - 1);
    set_brightness(g_lcd_device);

    //rt_device_close(g_lcd_device);
}

void nes_lcd_refresh(void)
{
    if (!g_lcd_device)
    {
        rt_kprintf("%s %d: no lcd dev\n", __func__, __LINE__);
        return;
    }

    if (!g_lcd_device->user_data)
    {
        rt_kprintf("%s %d: lcd no ops\n", __func__, __LINE__);
        return;
    }

    //rt_kprintf("%s %d\n", __func__, __LINE__);
    int32_t dx = (LCD_HOR_RES_MAX - NES_DISP_WIDTH) / 2;
    int32_t dy = (LCD_VER_RES_MAX - NES_DISP_HEIGHT) / 2;
    rt_graphix_ops(g_lcd_device)->draw_rect((const char *)lcd_buffer, dx, dy, dx + NES_DISP_WIDTH - 1, dy + NES_DISP_HEIGHT - 1);
    //rt_kprintf("%s %d\n", __func__, __LINE__);
}