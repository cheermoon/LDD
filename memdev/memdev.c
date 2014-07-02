#include <linux/module.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define MEMDEV_SIZE 1024
#define MEMDEV_MAJOR 0

#define LIX_26 0 //I compile the programm in Ubuntu 10.04 12.04LST 14.04LST

static int memdev_major = MEMDEV_MAJOR;

struct memdev_dev {
	struct cdev cdev;
	char mem[MEMDEV_SIZE];
};

loff_t memdev_llseek(struct file *fp, loff_t off, int ori)
{
	return 0;
}

ssize_t memdev_read(struct file *fp, char __user *buff, size_t count, loff_t *offp)
{
	return 0;
}

ssize_t memdev_write(struct file *fp, const char __user *buff, size_t count, loff_t *offp)
{
	return 0;
}
#if LIX_26
int memdev_ioctl(struct inode *nodep, struct file *fp, unsigned int cmd, unsigned long arg)
{
	return 0;
}
#endif
int memdev_open(struct inode *nodep, struct file *fp)
{
	return 0;
}


int memdev_release(struct inode *nodep, struct file *fp)
{
	return 0;
}

struct file_operations memdev_fops = {
	.owner = THIS_MODULE,
	.llseek = memdev_llseek,
	.read = memdev_read,
	.write = memdev_write,
#if LIX_26
	.ioctl = memdev_ioctl,
#endif
	.open = memdev_open,
	.release = memdev_release,
};

static void memdev_setup(struct memdev_dev *dev, int index)
{
	dev_t devno = MKDEV(memdev_major, index);
	int err;
	cdev_init(&dev->cdev, &memdev_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &memdev_fops;
	err = cdev_add(&dev->cdev, devno, 1);

	if (err)
	printk(KERN_ERR"erro : %s line:%d\n", __FUNCTION__, __LINE__);
}
static int __init memdev_init(void)
{
	int ret;
	dev_t devno;

	if (memdev_major) {
		devno = MKDEV(memdev_major, 0);
		ret = register_chrdev_region(devno, 1 , "memdev");
	} else {
		ret = alloc_chrdev_region(&devno, 0, 1, "memdev");
		memdev_major = MAJOR(devno);
	}
	
	if (ret < 0) {
		printk(KERN_WARNING"memdev:cann't get memdev_major %d\n", memdev_major);
		return ret;
	}

	struct memdev_dev *devp = kmalloc(sizeof(struct memdev_dev), GFP_KERNEL);//google the function
	if (!devp) {
		printk(KERN_ERR"err : %s line:%d\n", __FUNCTION__, __LINE__);
		unregister_chrdev_region(devno, 1);
		return -ENOMEM;
	}
	memset(dev, 0, sizeof(struct memdev_dev));

	memdev_setup(devp, 0);
	return 0;
}

static void __exit memdev_exit(void)
{
}


module_init(memdev_init);
module_exit(memdev_exit);

MODULE_AUTHOR("moon.cheng");
MODULE_VERSION("v0.1");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is a charactor device demo driver");

