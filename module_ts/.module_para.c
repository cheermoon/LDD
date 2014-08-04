#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>

static char *whom = "moon";
static int times = 3;
module_param(times, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);
static int __init mod_init(void)
{
	int i;
	for (i = 0; i < times; i++)
	printk(KERN_ALERT"%s", whom);
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

