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

static volatile struct lcd_regs m_lcd_regs;
static u32 pseudo_palette[16];


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

static int m_lcd_fb_setcolreg(unsigned regno, unsigned red, unsigned green,
			    unsigned blue, unsigned transp, struct fb_info *info) 
{
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
	m_lcd_info->screen_size		=	240*324*16/8;
}

static void m_lcd_gpio_init(void)
{
	
}


static void m_lcd_regs_init(struct lcd_regs *regs)
{
	
}

static int m_lcd_drv_probe(struct platform_device *p_dev)
{
	struct fb_info *m_lcd_info;
	
	/*1. Apply FBI structure init fix&var parameters*/
	m_lcd_info  = framebuffer_alloc(0, NULL);
	m_lcd_info_init(m_lcd_info);
	/*2. Init lcd controller*/
	
	/*3. Apply display buffer*/
	/*4. Register framebuffer */
	
	return 0;
}

static int m_lcd_drv_remove(struct platform_device *p_dev)
{	
	struct fb_info *m_lcd_info = dev_get_drvdata(&p_dev->dev);

	if (m_lcd_info) {
		unregister_framebuffer(m_lcd_info);
		//dealloc_dis_buffer();
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

