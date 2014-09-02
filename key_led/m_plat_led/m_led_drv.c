#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>


static int m_led_probe(struct platform_device *devp)
{
	return 0;
}
static int m_led_remove(struct platform_device *devp)
{
	return 0;
}

static struct platform_driver m_led_drv = {
			.probe      =    m_led_probe,
			.remove   =   m_led_remove,
			.driver      =   {
					.name = "m_led",
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

