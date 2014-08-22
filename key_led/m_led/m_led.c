#include <linux/init.h> //modules
#include <linux/module.h> //modules
#include <linux/cdev.h> //cdev
#include <linux/fs.h> //file_operations
#include <linux/ioport.h> //request_mem_region()
#include <asm/io.h> //ioremap()
#include <asm/uaccess.h> //copy_from_user()
#include <linux/device.h> //class

#define BASE_ADDR (0x56000050)
#define DRV_VERSION "0.9.4"


static struct class *led_cls;
static struct device *led_cls_dev;

struct led_dev {
	struct cdev cdev;
	volatile unsigned long *led_con;
	volatile unsigned char *led_dat;
	int major_no;
} *devp;

static ssize_t led_write(struct file *fp, const char __user *buff, size_t size, loff_t *fpos)
{
	unsigned long int tmp32;
	unsigned char tmp8; 
	unsigned char op = 0; 
	struct led_dev *devp = fp->private_data;

	/*size must be 1*/
	if (1 != size) {
		return -EFAULT;
	}
	/*read operations from user*/
	if (copy_from_user(&op, buff, 1)) {
		return  -EFAULT;
	}
	/*config gpio to output mode*/
	tmp32 = ioread32(devp->led_con);
	tmp32 &= ~(3<<8 | 3<<10 | 3<<12);
	tmp32 |= (1<<8 | 1<<10 | 1<<12);
	iowrite32(tmp32, devp->led_con);
	/*update gpio ports state*/
	tmp8 = ioread8(devp->led_dat);
	tmp8 &= ~(1<<4 | 1<<5 | 1<<6); //clear gpio
	tmp8 |= (op << 4); // for example op = 0b00000101 	
	iowrite8(tmp8, devp->led_dat);

	return 0;
}

static int led_close(struct inode *nd, struct file *fp)
{
	struct led_dev *devp = fp->private_data;

	iounmap(devp->led_con);
	release_mem_region(BASE_ADDR, 8);

	return 0;
}

static int led_open(struct inode *nd, struct file *fp)
{
	struct led_dev *devp;
	devp = container_of(nd->i_cdev, struct led_dev, cdev);
	fp->private_data = devp;
	
	if (!request_mem_region(BASE_ADDR, 8, "led_io")) {
		return -ENOMEM;
	}
	
	devp->led_con = ioremap(BASE_ADDR, 8); //phy_addr 0x56000050
	devp->led_dat = (volatile unsigned char *)(devp->led_con + 1); //pht_addr 0x56000054

	return 0;
}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
	.write = led_write,
};


static int __init led_init(void)
{
	int ret;
	dev_t dev_no;

	ret = alloc_chrdev_region(&dev_no, 0, 1, "led_drv");  
	if (!!ret) {
		 printk(KERN_ERR"arrage device no erro!\n");  
		 return ret;
	}

	devp = kmalloc(sizeof(struct led_dev), GFP_KERNEL);
	if (!devp) {
		ret = -ENOMEM;
		goto out1;//goto !!
	}
	memset(devp, 0, sizeof(struct led_dev));
	
	/*init led_dev*/
	devp->major_no = MAJOR(dev_no);
	cdev_init(&devp->cdev, &led_fops);
	devp->cdev.owner = THIS_MODULE;
	ret = cdev_add(&devp->cdev, dev_no, 1);
    if(ret) {	
		goto out2;//goto !!
	}
	
	led_cls = class_create(THIS_MODULE, "led_drv");
	led_cls_dev = device_create(led_cls, NULL, dev_no, "m_led");
	
	return 0; //init successful

out2 :
	kfree(devp);	
out1 :
	unregister_chrdev_region(dev_no, 1);	
	return ret;//init failed
}

static void __exit led_exit(void)
{
	device_destroy(led_cls, MKDEV(devp->major_no, 0));
	class_destroy(led_cls);
	cdev_del(&devp->cdev);
	unregister_chrdev_region(MKDEV(devp->major_no, 0),1);
	kfree(devp);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("moon.cheng.2014@gmail.com");
MODULE_DESCRIPTION("this is a led drv for JZ2440");
MODULE_VERSION(DRV_VERSION);
