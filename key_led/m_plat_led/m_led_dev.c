/*  
	key_led/m_plat_led/m_led_dev.c : ldd
 
	   Author : moon.cheng.2014@gmail.com  
	   Date   : 2014-09-09
	   Version: 0.1.0
			   
	   This program is a demo program for linux device drivers created by moon. 
	   It is a key driver for JZ2440.  You can learn the platform device subsystem 
	   of linux by this program. 
				   
	   If you find some bugs or you have some advices. Send me a email pls!
					   
	   This program is free software; you can redistribute it and/or modify
	   it under the terms of the GNU General Public License as published by
	   the Free Software Foundation; either version 2 of the License, or
	   (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static void	m_led_release(struct device *dev)
{
	/*we do nothing here, but if do not have this function
	    we will meet an error when compiling the program*/
}

/*the resource of our leds in JZ2440*/
static struct resource m_led_rsc[] = {
			[0] = {
        				.start = 0x56000050,
        				.end   = 0x56000050 + 8 - 1,
        				.flags = IORESOURCE_MEM,
			},
			[1] = {
				.start = 4,
				.end   = 6,
				.flags = IORESOURCE_IRQ,
			}
};

/*our led device struct*/
static struct platform_device m_led_dev = {
	.name                      =	"m_led",
	.id                             =	-1,
	.num_resources  =	ARRAY_SIZE(m_led_rsc),
	.resource               =	m_led_rsc,
	.dev                         =	{
						.release  = m_led_release,
					}
};
static int __init m_led_dev_init(void)
{
	platform_device_register(&m_led_dev);
	return 0;
}

static void __exit m_led_dev_exit(void)
{
	platform_device_unregister(&m_led_dev);
}

module_init(m_led_dev_init);
module_exit(m_led_dev_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("moon.cheng.2014@gmail.com");

