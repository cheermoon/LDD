#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define BASE_ADDR (0x56000050)

struct led_dev {
	struct cdev cdev;
	unsigned long int *led_con;
	unsigned long int *led_dat;
}

static int __init led_init(void)
{
	int ret;
	dev_t dev_no;

	ret = alloc_chrdev_region(&dev_no, 0, 1, "memdev");  
	if (!!ret) {
		 printk(KERN_ERR"arrage device no erro!\n");  
		 return ret;
	}


	return 0;
}

static void __exit led_exit(void)
{
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL v2");
