/*
	m_key.c : LDD

	
  */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <asm/gpio.h> /*S3C2410_GPF0 .. */
#include <linux/cdev.h>
#include <linux/device.h> /*class */
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/irq.h> /*IRQ NO*/
#include <linux/interrupt.h> /*request_irq free_irq*/

#define MAJOR_NO 0
/*the gpio description struct*/
struct pin_desc{
	unsigned int pin; /*GPIO*/
	unsigned int key_val; /*pin status*/
};

/*the key device struct*/
struct m_key_dev {
	struct cdev cdev; 
	struct pin_desc key_desc;
	wait_queue_head_t key_wq;
	int key_down;
};
/*
static struct pin_desc pins_desc[4] = {
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04},
};
*/
static struct m_key_dev *devp;
static int major_no;
static struct class  *m_key_clsp;
static struct device *m_key_devp;  

static irqreturn_t m_key_irq_handle(int irq, void *dev)
{
	
	return IRQ_HANDLED;
}
static ssize_t m_key_read(struct file *fp, char __user *buf, size_t size, loff_t *f_pos)
{
	struct m_key_dev *devp = fp->private_data;

	wait_event_interruptible(devp->key_wq, devp->key_down);
	//struct pin_desc pinval = s3c2410_gpio_getpin(devp->key_desc.pin);
	return 0;
}

static int m_key_open(struct inode *nd, struct file *fp)
{
	/*you should init fp->private_data by yourself*/
	struct m_key_dev *devp ;
	int ret;
	fp->private_data = container_of(nd->i_cdev, struct m_key_dev, cdev);
	devp = fp->private_data;
	/*apply IRQ do not process exception*/
	ret = request_irq(IRQ_EINT0, m_key_irq_handle, IRQ_TYPE_EDGE_BOTH,  "key1", devp);
	ret = request_irq(IRQ_EINT2, m_key_irq_handle, IRQ_TYPE_EDGE_BOTH,  "key2", devp);
	ret = request_irq(IRQ_EINT11, m_key_irq_handle, IRQ_TYPE_EDGE_BOTH,  "key3", devp);
	ret = request_irq(IRQ_EINT19, m_key_irq_handle, IRQ_TYPE_EDGE_BOTH,  "key4", devp);
	return 0;
}

static int m_key_close(struct inode *nd, struct file *fp)
{
	free_irq(IRQ_EINT0, devp);
	free_irq(IRQ_EINT2, devp);
	free_irq(IRQ_EINT11, devp);
	free_irq(IRQ_EINT19, devp);
	
	return 0;
}

static struct file_operations m_key_fpos  = {
		.open     = m_key_open,
		.read      = m_key_read,
		.release  = m_key_close,	
};

static int __init m_key_init(void)
{
	dev_t devno;
	int ret;

	if (MAJOR_NO) {
		devno = MKDEV(MAJOR_NO, 0);
		ret = register_chrdev_region(devno,  1, "m_keys_drv");
	} else {
		ret = alloc_chrdev_region(&devno, 0, 1, "m_keys_drv");
	}
		
	if (ret) { 
		goto out;
	}

	/*allocate memory to device struct*/
	devp = kmalloc(sizeof(struct m_key_dev), GFP_KERNEL);
	if (!devp) {
		goto out1;
	}
	memset(devp, 0, sizeof(struct m_key_dev)); /*init the space to zero*/

	major_no = MAJOR(devno);
	cdev_init(&devp->cdev, &m_key_fpos);
	devp->cdev.owner = THIS_MODULE;
	ret = cdev_add(&devp->cdev, devno, 1);
	if (ret) {
		goto out2;
	}

	/*auto create devices files in /dev */
	m_key_clsp = class_create(THIS_MODULE, "m_keys");
	m_key_devp = device_create(m_key_clsp,  NULL, devno, NULL, "m_button%d", 1); /*/dev/m_button*/
	if (!m_key_devp) { /*if can not create a device file */
		goto out3;
	}

	init_waitqueue_head(&devp->key_wq); /*init the wait queue*/
	return 0;

	out3:
		class_destroy(m_key_clsp);
	out2:
		kfree(devp);
	out1:
		unregister_chrdev_region(major_no, 1);
	out:
		return ret;
}

static void __exit m_key_exit(void)
{
	device_destroy(m_key_clsp, MKDEV(major_no, 0));
	class_destroy(m_key_clsp);
	cdev_del(&devp->cdev);
	kfree(devp);
	unregister_chrdev_region(major_no, 1);
}

module_init(m_key_init);
module_exit(m_key_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("moon.cheng.2014@gmail.com");
MODULE_DESCRIPTION("a gpio key driver for JZ2440");
MODULE_VERSION("0.95");

