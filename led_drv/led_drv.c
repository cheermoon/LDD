#include <linux/init.h> //modules
#include <linux/module.h> //modules
#include <linux/cdev.h> //cdev
#include <linux/fs.h> //file_operations
#include <linux/ioport.h> //request_mem_region()
#include <asm/io.h> //ioremap()
#include <asm/uaccess.h> //copy_from_user()
#include <linux/device.h> //class

#define BASE_ADDR (0x56000050)

struct led_dev {
	struct cdev cdev;
	volatile unsigned long *led_con;
	volatile unsigned long *led_dat;
	int major_no;
} *devp;

static ssize_t led_write(struct file *fp, const char __user *buff, size_t size, loff_t *fpos)
{
	unsigned long int tmp;
	struct led_dev *devp = fp->private_data;
	int op = 0;

	/*read operations from user*/
	if (copy_from_user(&op, buff, 4)) {
		return  -EFAULT;
	}
	/*config gpio to output mode*/
	tmp = ioread32(devp->led_con);
	tmp &= ~(3<<8 | 3<<10 | 3<<12);
	tmp |= (1<<8 | 1<<10 | 1<<12);
	iowrite32(tmp, devp->led_con);
	/*update gpio ports state*/
	tmp = ioread32(devp->led_dat);
	if (1 == op) {
		tmp &= ~(1<<4 | 1<<5 | 1<<6);
	} else {
		tmp |= (1<<4 | 1<<5 | 1<<6);
	}
	iowrite32(tmp, devp->led_dat);

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
	devp->led_dat = devp->led_con + 1; //pht_addr 0x56000054

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
	
	return 0; //init successful

out2 :
	kfree(devp);	
out1 :
	unregister_chrdev_region(dev_no, 1);	
	return ret;//init failed
}

static void __exit led_exit(void)
{
	cdev_del(&devp->cdev);
	unregister_chrdev_region(MKDEV(devp->major_no, 0),1);
	kfree(devp);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL v2");
