#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define BASE_ADDR (0x56000050)

struct led_dev {
	struct cdev cdev;
	unsigned long int *led_con;
	unsigned long int *led_dat;
	int major_no;
} *devp;

struct file_operations led_fops = {
	.open = led_open,
	.release = led_close,
	.write = led_write,
};


static int __init led_init(void)
{
	int ret;
	dev_t dev_no;

	ret = alloc_chrdev_region(&dev_no, 0, 1, "led_dev");  
	if (!!ret) {
		 printk(KERN_ERR"arrage device no erro!\n");  
		 return ret;
	}
	devp->major_no = MAJOR(dev_no);

	devp = kmalloc(sizeof(struct led_dev), GFP_KERNEL);
	if (!devp) {
		ret = -ENOMEM;
		goto out1;
	}
	memset(devp, 0, sizeof(struct led_dev));
	
	/*init led_dev*/
	cdev_init(&devp->cdev, &led_fops);
	devp->cdev.owner = THIS_MODULE;
	ret = cdev_add(&devp->cdev, dev_no, 1);
    if(ret) {	
		goto out2;
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
