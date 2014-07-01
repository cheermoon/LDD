#include <linux/module.h>
#include <linux/init.h>

static int __init mod_init(void)
{
	printk(KERN_ALERT"init");
	return 0;
}

static void __exit mod_exit(void)
{
	printk(KERN_ALERT"exit");
}


module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("moon.cheng");
MODULE_VERSION("v0.1");
MODULE_DESCRIPTION("this is just a demo program");

