/*  
	m_input_key.c : ldd
 
	   Author : moon.cheng.2014@gmail.com  
	   Date   : 2014-08-26
	   Version: 0.1.0
			   
	   This program is a demo program for linux device drivers created by moon. 
	   It is a key driver for JZ2440.  You can learn the input subsystem of linux 
	   by this program. 
				   
	   If you find some bugs or you have some advices. Send me a email pls!
					   
	   This program is free software; you can redistribute it and/or modify
	   it under the terms of the GNU General Public License as published by
	   the Free Software Foundation; either version 2 of the License, or
	   (at your option) any later version.
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h> /*input subsys*/
#include <linux/irq.h> /*IRQT_BOTHEDGE etc*/
#include <linux/interrupt.h> /*request_irq & free_irq*/
#include <asm/gpio.h> /*s3c2410_gpio_getpin*/

#define DRV_VERSION "0.1.0"

/*our private struct to describe a key*/
struct key_desc{
	int irq;  /*irq NO*/
	char *name; /*key name */
	unsigned int pin; /*GPIO pin*/
	unsigned int key_val; /*Key code*/
};

/*declare a input device*/
static struct input_dev *m_key_input_p;

/*we have 4 keys in JZ2440*/
struct key_desc key_desc[4] = {
	{IRQ_EINT0,  "m_key_l", S3C2410_GPF0,   KEY_L},
	{IRQ_EINT2,  "m_key_s", S3C2410_GPF2,   KEY_S},
	{IRQ_EINT11, "m_key_ent", S3C2410_GPG3,   KEY_ENTER},
	{IRQ_EINT19, "m_key_lsft",  S3C2410_GPG11, KEY_LEFTSHIFT},
};

/*IQR Handler*/
static irqreturn_t  m_key_irq_handler(int irq, void *dev_id)
{	
	struct key_desc *key_desc;
	int pin_val;

	key_desc = (struct key_desc *)dev_id;
	pin_val = s3c2410_gpio_getpin(key_desc->pin);

	/*report the event*/
	if (pin_val) { /*the key is up */
		input_event(m_key_input_p, EV_KEY, key_desc->key_val, 0);
		input_sync(m_key_input_p); /*don't forget it*/
	} else { /*the key is down*/
		input_event(m_key_input_p, EV_KEY, key_desc->key_val, 1);
		input_sync(m_key_input_p);
	}
	
	return IRQ_HANDLED; 
}

static int __init m_input_key_init(void)
{
	int i, j, ret;

	/*get a input device*/
	m_key_input_p = input_allocate_device();
	if (!m_key_input_p) {
		return   -ENOMEM;
	}

	/*set event types*/
	set_bit(EV_KEY, m_key_input_p->evbit);
	set_bit(EV_REP, m_key_input_p->evbit); /*rep when long press the key*/

	/*set key code*/
	set_bit(KEY_L, m_key_input_p->keybit);
	set_bit(KEY_S, m_key_input_p->keybit);
	set_bit(KEY_ENTER, m_key_input_p->keybit);
	set_bit(KEY_LEFTSHIFT, m_key_input_p->keybit);

	/*request the irq, you must check the return value*/
	for (i = 0; i < 4; i++) {
		ret = request_irq(key_desc[i].irq, m_key_irq_handler, IRQT_BOTHEDGE, key_desc[i].name, &key_desc[i]);
		if (ret < 0) { /*if request failed*/
			for (j =0; j < i; j++) {
				free_irq(key_desc[i].irq, &key_desc[i]); /*free the requested irq*/
			}
			return  ret;
		}
	}

	/*register input device*/
	ret = input_register_device(m_key_input_p);
	return 0;
}

static void __exit m_input_key_exit(void)
{
	
	int i;
	/*free irq*/
	for (i = 0; i < 4; i++) {
			free_irq(key_desc[i].irq, &key_desc[i]);
	}

	/*unregister input device*/
	input_unregister_device(m_key_input_p);
	input_free_device(m_key_input_p);
}

module_init(m_input_key_init);
module_exit(m_input_key_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("moon.cheng.2014@gmail.com ");
MODULE_VERSION(DRV_VERSION);


