/*  
	m_lcd/m_lcd_dev.c : ldd
 
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
#include <linux/platform_device.h>

#include "m_lcd.h"


#define DRV_VERSION "0.1.0"

static void	m_lcd_release(struct device *dev)
{
	/*we do nothing here, but if do not have this function
	    we will meet an error when compiling the program*/
}

/*the resource of our leds in JZ2440*/
static struct resource m_lcd_rsc[] = {
			[0] = {
        				.start = 0x4d000000,
        				.end   = 0x4d000000 + sizeof(struct lcd_regs) - 1,
        				.flags = IORESOURCE_MEM,
			}
};

/*our led device struct*/
static u64 m_lcd_dma_mask = 0xffffffffUL;
static struct platform_device m_lcd_dev = {
	.name                      =	"m_lcd",
	.id                             =	-1,
	.num_resources  =	ARRAY_SIZE(m_lcd_rsc),
	.resource               =	m_lcd_rsc,
	.dev                         =	{
						.release  = m_lcd_release,
						.dma_mask = &m_lcd_dma_mask,
						.coherent_dma_mask = 0xffffffffUL,
					}
};

static int __init m_lcd_dev_init(void)
{
	return platform_device_register(&m_lcd_dev);
}

static void __exit m_lcd_dev_exit(void)
{
	platform_device_unregister(&m_lcd_dev);
}

module_init(m_lcd_dev_init);
module_exit(m_lcd_dev_exit);

MODULE_AUTHOR("moon.cheng.2014@gmail.com");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION("this is a lcd test driver created by moon.cheng");
