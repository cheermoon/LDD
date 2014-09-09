/*  
	key_led/m_plat_led/m_led_drv.c : ldd
 
	   Author : moon.cheng.2014@gmail.com  
	   Date   : 2014-09-09
	   Version: 0.1.0
			   
	   This program is a demo program for linux device drivers created by moon. 
	   It is a key driver for JZ2440.  You can learn the platform driver subsystem 
	   of linux by this program. 
				   
	   If you find some bugs or you have some advices. Send me a email pls!
					   
	   This program is free software; you can redistribute it and/or modify
	   it under the terms of the GNU General Public License as published by
	   the Free Software Foundation; either version 2 of the License, or
	   (at your option) any later version.	   
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <linux/uaccess.h>

struct m_led_device {
		struct cdev cdev;
		struct resource res[2];
		volatile unsigned long *con;
		volatile unsigned char *dat;
		int major;
};

static struct m_led_device  *m_led_devp;
static struct class *m_led_clsp;
static struct device *m_led_cls_devp;

static int m_led_open(struct inode *nd, struct file *fp)
{
	int ret = 0;
	struct resource *res;
	/*init the private_data member*/
	fp ->private_data = container_of(nd->i_cdev, struct m_led_device, cdev);

	res = &m_led_devp->res[0];
	if (!request_mem_region(res->start, res->end - res->start +1, "led_io") ){
		ret =  -ENOMEM;
		goto OUT;
	}	
	m_led_devp->con = (volatile unsigned long *)ioremap(res->start, res->end - res->start +1);
	m_led_devp->dat = (volatile unsigned char *)(m_led_devp->con + 1);

OUT:
	return ret;
}

static int m_led_close (struct inode *nd, struct file *fp)
{	
	struct resource *res;
	
	/*release reosurce*/	
	res = &m_led_devp->res[0];
	iounmap(m_led_devp->con);
	release_mem_region(res->start, res->end - res->start +1);

	return 0;
}

static ssize_t m_led_write(struct file *fp, const char __user *buf, size_t size, loff_t *pos)
{	
	unsigned long int tmp32;
	unsigned char tmp8; 
	unsigned char op = 0; 
	unsigned char op_mask = 0; /*operation mask*/
	int i; /*temp var*/
	struct resource *res;

	/*size must be 1*/
	if (1 != size) {
		return -EFAULT;
	}
	/*read operations from user*/
	if (copy_from_user(&op, buf, 1)) {
		return  -EFAULT;
	}

	res = &m_led_devp->res[1];
	/*config gpio to output mode*/
	tmp32 = ioread32(m_led_devp->con);	
	for (i = res->start; i <= res->end; i++) {
		tmp32 &= ~(3 << (2 * i));
		tmp32 |= (1 << (2 * i));
	}
	iowrite32(tmp32, m_led_devp->con);
	
	/*update gpio ports state*/
	tmp8 = ioread8(m_led_devp->dat);
	for (i = res->start; i <= res->end; i++) {
		tmp8 &= ~(1 << i); //clear gpio 
		op_mask |= (1 << i); //set operate mask
	}	
	op <<= res->start; // for example op = 0b00000101 
	op &= op_mask; //avoid to operate other gpios 
	tmp8 |= op; 
	iowrite8(tmp8, m_led_devp->dat);

	return 0;
}

/*same as the char device drivers*/
static  struct file_operations m_led_fops = {
				.owner   = THIS_MODULE,
				.open      = m_led_open,
				.write      =  m_led_write,
				.release   =  m_led_close
};

/*many works are moved to here from the init function & when the sys find a device 
   match this driver, this function will be invoked*/
static int m_led_probe(struct platform_device *devp)
{
	int ret;
	dev_t devno;
	struct resource *res;

	/*get device number*/
	ret = alloc_chrdev_region(&devno, 0,  1, "m_led_drv");
	if (!!ret) 
		goto OUT;
	
	m_led_devp = kmalloc(sizeof(struct m_led_device), GFP_KERNEL);
	if (!m_led_devp) {
		ret = -ENOMEM;
		goto FAILED_GET_MEM;
	}
	memset(m_led_devp, 0, sizeof(struct m_led_device));

	/*register char device*/
	m_led_devp->major = MAJOR(devno);
	cdev_init(&m_led_devp->cdev, &m_led_fops);
	m_led_devp->cdev.owner = THIS_MODULE;
	ret = cdev_add(&m_led_devp->cdev, devno, 1);
	if (!!ret)
		goto FAILED_ADD_CDEV;
	
	/*get resources from the device*/
	res = platform_get_resource(devp, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -EBUSY;
		goto FAILED_GET_RES;
	}	
	m_led_devp->res[0] = *res;
	res = platform_get_resource(devp, IORESOURCE_IRQ, 0);
	if (!res) {
		ret = -EBUSY;
		goto FAILED_GET_RES;
	}
	m_led_devp->res[1] = *res;
	
	/*creat device file /dev/m_led*/
	m_led_clsp = class_create(THIS_MODULE, "m_led_drv");
	m_led_cls_devp = device_create(m_led_clsp, NULL, devno,"m_led");
	if(!m_led_cls_devp) 
		goto FAILED_CREAT_DEV;

	/*success return*/
	return 0;

/*fail return*/	
FAILED_CREAT_DEV:
FAILED_GET_RES:	
	cdev_del(&m_led_devp->cdev);
FAILED_ADD_CDEV:
	kfree(m_led_devp);
FAILED_GET_MEM:
	unregister_chrdev_region(devno, 1);
OUT:
	return ret;
}

static int m_led_remove(struct platform_device *devp)
{	
	/*destroy device file*/
	device_destroy(m_led_clsp,MKDEV(m_led_devp->major, 0));
	class_destroy(m_led_clsp);

	/*unregister device*/
	cdev_del(&m_led_devp->cdev);
	unregister_chrdev_region(MKDEV(m_led_devp->major, 0), 1);
	kfree(m_led_devp);
	
	return 0;
}

static struct platform_driver m_led_drv = {
			.probe      =    m_led_probe,
			.remove   =   m_led_remove,
			.driver      =   {
					.name = "m_led", /*the name must match the device name*/
					.owner = THIS_MODULE,	
			}
};

static int __init m_led_drv_init(void)
{
	platform_driver_register(&m_led_drv);
	return 0;
}

static void __exit m_led_drv_exit(void)
{
	platform_driver_unregister(&m_led_drv);
}

module_init(m_led_drv_init);
module_exit(m_led_drv_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("moon.cheng.2014@gmail.com");

