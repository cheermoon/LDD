/*  
	m_lcd/m_lcd_drv.c : ldd
 
	   Author : moon.cheng.2014@gmail.com  
	   Date   : 2014-09-30
	   Version: 0.1.0
			   
	   This program is a demo program for linux device drivers created by moon. 
	   It is a key driver for JZ2440.  You can learn something about the lcd device 
	   driver in this program. 
				   
	   If you find some bugs or you have some advices. Send me a email pls!
					   
	   This program is free software; you can redistribute it and/or modify
	   it under the terms of the GNU General Public License as published by
	   the Free Software Foundation; either version 2 of the License, or
	   (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>

#define DRV_VERSION "0.1.0"

struct lcd_regs {
		unsigned long	lcdcon1;
		unsigned long	lcdcon2;
		unsigned long	lcdcon3;
		unsigned long	lcdcon4;
		unsigned long	lcdcon5;
    		unsigned long	lcdsaddr1;
    		unsigned long	lcdsaddr2;
    		unsigned long	lcdsaddr3;
    		unsigned long	redlut;
    		unsigned long	greenlut;
    		unsigned long	bluelut;
		unsigned long	reserved[9];
    		unsigned long	dithmode;
		unsigned long	tpal;
		unsigned long	lcdintpnd;
		unsigned long	lcdsrcpnd;
		unsigned long	lcdintmsk;
		unsigned long	lpcsel;
};

static volatile struct lcd_regs *m_lcd_regs;
static u32 pseudo_palette[16];

static volatile unsigned long *gpio_b_con;
static volatile unsigned long *gpio_b_dat;
static volatile unsigned long *gpio_c_con;
static volatile unsigned long *gpio_d_con;
static volatile unsigned long *gpio_g_con;

static struct fb_fix_screeninfo m_lcd_fb_fix  = {
	.id			=	"m_lcd",
	.smem_len		=	240*320*16/8,
	.type			=	FB_TYPE_PACKED_PIXELS,
	.visual		=	FB_VISUAL_TRUECOLOR,  /* TFT */
	.line_length	=	240*2,
};

static struct fb_var_screeninfo m_lcd_fb_var  = {
	.xres			=	240,
	.yres			=	320,
	.xres_virtual	=	240,
	.yres_virtual	=	320,
	.bits_per_pixel	=	16,

	/* RGB:565 */
	.red.offset		=	11, 	 /*red*/
	.red.length		=	5,
	.green.offset	=	5,	/*green*/
	.green.length	=	6,
	.blue.offset 	=	0,	/*blue*/
	.blue.length	=	5,
	.activate		=	FB_ACTIVATE_NOW,
};


static inline unsigned int m_chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int m_lcd_fb_setcolreg(unsigned regno, unsigned red, unsigned green,
			    unsigned blue, unsigned transp, struct fb_info *info) 
{
	unsigned int val;
	u32 *pal = info->pseudo_palette;

	
	if (regno < 16) 
		return 1;
	
	val	=	m_chan_to_field(red,   &info->var.red);
	val	|=	m_chan_to_field(green, &info->var.green);
	val	|=	m_chan_to_field(blue,  &info->var.blue);

	pal[regno] = val;
		
	return 0;
}


static struct fb_ops m_lcd_fb_ops = {
	.owner		=	THIS_MODULE,
	.fb_setcolreg	=	m_lcd_fb_setcolreg,
	.fb_fillrect	           =	cfb_fillrect,
	.fb_copyarea	=	cfb_copyarea,
	.fb_imageblit	=	cfb_imageblit,
};

static void m_lcd_info_init(struct fb_info *m_lcd_info)
{
	/*Set fix parameters*/
	m_lcd_info->fix	=	m_lcd_fb_fix;
	/*Set variable parameters*/
	m_lcd_info->var	=	m_lcd_fb_var;
	/*Set Operation function*/
	m_lcd_info->fbops	=	&m_lcd_fb_ops;
		
	/*Other settings*/
	m_lcd_info->pseudo_palette	=	pseudo_palette;
	//s3c_lcd->screen_base  = ;
	m_lcd_info->screen_size		=	240*324*16/8;    //bytes
	/*3. Apply display buffer*/
	m_lcd_info->screen_base = dma_alloc_writecombine(m_lcd_info->dev, m_lcd_info->fix.smem_len, (dma_addr_t*)&(m_lcd_info->fix.smem_start) , GFP_KERNEL);
}

static int m_lcd_gpio_map(void)
{
	int ret =0;

	if (!request_mem_region(0x56000010,  8, "gpio_b_con") ){
		ret =  -ENOMEM;
		goto OUT;
	}	
	if (!request_mem_region(0x56000020,  4, "gpio_c_con") ){
		ret =  -ENOMEM;
		goto OUT;
	}	
	if (!request_mem_region(0x56000030,  4, "gpio_d_con") ){
		ret =  -ENOMEM;
		goto OUT;
	}	
	if (!request_mem_region(0x56000040,  4, "gpio_g_con") ){
		ret =  -ENOMEM;
		goto OUT;
	}	
	
	gpio_b_con = (volatile unsigned long *)ioremap(0x56000010, 8);
	gpio_b_dat = gpio_b_con +1;
	gpio_c_con = (volatile unsigned long *)ioremap(0x56000020, 4);
	gpio_d_con = (volatile unsigned long *)ioremap(0x56000030, 4);
	gpio_g_con = (volatile unsigned long *)ioremap(0x56000040, 4);

OUT:
	return ret;
	
}

static void m_lcd_gpio_unmap(void)
{
	iounmap(gpio_b_con);
	release_mem_region(0x56000010,  8);
	iounmap(gpio_c_con);
	release_mem_region(0x56000020,  4);
	iounmap(gpio_d_con);
	release_mem_region(0x56000030,  8);
	iounmap(gpio_g_con);
	release_mem_region(0x56000040,  8);
}
static int m_lcd_gpio_init(void)
{
	int ret =0;

	ret = m_lcd_gpio_map();
	if (ret) 
		goto OUT;
	
	/*set gpios*/
	*gpio_c_con = 0xaaaaaaaa; /*lcd controller data sheet*/
	*gpio_d_con = 0xaaaaaaaa; /*lcd controller data sheet*/
	
	*gpio_g_con |= (3<<8);/*LCD_PWRDN*/
	
	/*open back light*/
	*gpio_b_con &= ~3;
	*gpio_b_con |= 1;
	*gpio_b_dat &= ~1;
	*gpio_b_dat |= 1;


OUT:
	return ret;
}

static void m_lcd_gpio_release(void)
{
	m_lcd_gpio_unmap();
}



static int  m_lcd_regs_init(struct resource *res, struct fb_info *m_lcd_info)
{
	int ret = 0;
	
	if (!request_mem_region(res->start, res->end - res->start +1, "lcd_regs") ){
		ret =  -ENOMEM;
		goto OUT;
	}
	m_lcd_regs = (volatile struct lcd_regs *)ioremap(res->start, res->end - res->start +1);

	/*	m_lcd_regs->lcdcon1
	 *	LINECNT(read only) [27:18]  [0000000000]
	 *	CLKVAL [17:8] VCLK = HCLK/[(CLKVAL+1)x2]
	 				   10M = 60M /[(CLKVAL + 1) x 2]  ---> CLKVAL=2
	 *	MMODE [7] 0
	 *	PNRMODE [6:5] [11]=3 TFT LCD
	 *	BPPMODE [4:1] [1100]=12 TFT 16bpp for TFT
	 *	ENVID [0] [0]=0
	 */
	m_lcd_regs->lcdcon1 = (2<<8) | (3<<5) | (12 <<1);

	/*	m_lcd_regs->lcdcon2
	 *	VBPD [31:24] VBPD+1 = T0-T2-T1 = 327-322-1 -->VBPD=3
	 *	LINEVAL [23:14] 320 -1 = 319  320 lines
	 *	VFPD [13:6] VFPD+1 = T2 - T5 = 322 -320 -->VFPD = 1
	 *	VSPW [5:0] VSPW + 1 = T1 = 1 -->VSPW = 0
	 */
	 m_lcd_regs->lcdcon2 = (3<<24) | (319<<14) | (1<<6) ;
	
	/*	m_lcd_regs->lcdcon3
	 *	HBPD [25:19] HBPD+1 = T6-T7-T8 = 17  --> HBPD=16
	 *	HOZVAL [18:8] HOZVAL+1 = 240 -->HOZVAL=239
	 *	HFPD [7:0] HFPD+1 = T8-T11 = 11 -->HFPD=10 
	 */
	 m_lcd_regs->lcdcon3 = (16<<19) | (239<<8) | (10<<0);

	 /*	m_lcd_regs->lcdcon4
	  *	HSPW [7:0] HSPW+1 = T7 = 5
	  */
	  m_lcd_regs->lcdcon4 = 4;

	 /*	m_lcd_regs->lcdcon5
	  *	VSTATUS [16:15]	read only
	  *	HSTATUS [14:13]	read only
	  *	BPP24BL [12]	0-->LSB
	  *	FRM565 [11]		1-->565
	  *	INVVCLK [10]	0-->date fetched at falling edge (LCD Spec)
	  *	INVVLINE [9]		1-->HSYNC valid in low 
	  *	INVVFRAME [8]	1-->VSYNC valid in low
	  *	INVVD [7]		0-->video data polarity normal
	  *	INVVDEN [6]		0-->VDEN polarity normal
	  *	INVVPWREN [5]	0-->PWREN polarity normal
	  *	INVLEND [4]		0-->no LEND in this lcd
	  *	PWREN	[3]		0-->no PWREN in this lcd
	  *	ENLEND [2]		0->no LEND in this lcd
	  *	BSWP [1]		0-->lcd controller datasheet
	  *	HWSWP [0]		1-->lcd controller datasheet
	  */
	  m_lcd_regs->lcdcon5 = (1<<11) |(1<<9) |(1<<8) |(1<<0);

	m_lcd_regs->lcdsaddr1 =  (m_lcd_info->fix.smem_start>>1) & ~(3<<30);
	m_lcd_regs->lcdsaddr2 = ((m_lcd_info->fix.smem_start+m_lcd_info->fix.smem_len) >>1) & 0x1fffff;
	m_lcd_regs->lcdsaddr3 = (240*16/16);

	m_lcd_regs-> lcdcon1 |= 1;
	m_lcd_regs-> lcdcon5 |= (1<<3);
	
	 m_lcd_gpio_init();
OUT:
	return ret;
}

static void m_lcd_regs_release(struct resource *res)
{
	iounmap(m_lcd_regs);
	release_mem_region(res->start, res->end - res->start +1);
}


static int m_lcd_drv_probe(struct platform_device *p_dev)
{
	int ret;
	struct fb_info *m_lcd_info;
	struct resource *res;
	
	/*1. Apply FBI structure init fix&var parameters*/
	/*m_lcd_info->device = &p_dev->dev*/
	m_lcd_info  = framebuffer_alloc(sizeof(struct fb_info), &p_dev->dev);
	m_lcd_info->dev = &p_dev->dev;
	m_lcd_info_init(m_lcd_info);
	
	/*2. Init lcd controller*/
	/*Get resources from platform device*/
	res = platform_get_resource(p_dev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -EBUSY;
		goto FAILED_GET_RES;
	}
	m_lcd_regs_init(res, m_lcd_info);
	
	/*4. Register framebuffer */
	/*dev_set_drvdata(fb_info->dev, fb_info)*/
	register_framebuffer(m_lcd_info);
	return 0;
FAILED_GET_RES:
	return ret;
}

static int m_lcd_drv_remove(struct platform_device *p_dev)
{	
	struct resource *res;
	struct fb_info *m_lcd_info = dev_get_drvdata(&p_dev->dev);

	/*close lcd*/
	m_lcd_regs->lcdcon1 &= ~(1<<0);
	/*close back light*/
	*gpio_b_dat &= ~1;  
		
	if (m_lcd_info) {
		unregister_framebuffer(m_lcd_info);
		dma_free_writecombine(m_lcd_info->dev, m_lcd_info->fix.smem_len, m_lcd_info->screen_base, m_lcd_info->fix.smem_start);
		res = platform_get_resource(p_dev, IORESOURCE_MEM, 0);
		m_lcd_regs_release(res);
		m_lcd_gpio_release();
		framebuffer_release(m_lcd_info);
	}
	
	return 0;
}

struct platform_driver m_lcd_drv = {
	.probe = m_lcd_drv_probe,
	.remove = m_lcd_drv_remove,
	.driver = {
		.name = "m_lcd",
		.owner = THIS_MODULE,
	}
};
static int __init m_lcd_drv_init(void)
{
	return platform_driver_register(&m_lcd_drv);
}

static void __exit m_lcd_drv_exit(void)
{
	platform_driver_unregister(&m_lcd_drv);
}

module_init(m_lcd_drv_init);
module_exit(m_lcd_drv_exit);

MODULE_AUTHOR("moon.cheng.2014@gmail.com");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION("this is a lcd test driver created by moon.cheng");

