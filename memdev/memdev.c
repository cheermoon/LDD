#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define MEMDEV_SIZE 1024
#define MEMDEV_MAJOR 0

static int memdev_major = MEMDEV_MAJOR;

static struct memdev_dev {
	struct cdev cdev;
	char mem[MEMDEV_SIZE];
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
		memdev = MAJOR(devno);
	}
	

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

