#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

static void	m_led_release(struct device *dev)
{
	
}


static struct resource m_led_rsc[] = {
			[0] = {
        				.start = 0x56000050,
        				.end   = 0x56000050 + 8 - 1,
        				.flags = IORESOURCE_MEM,
			},
			[1] = {
				.start = 5,
				.end   = 5,
				.flags = IORESOURCE_IRQ,
			}
};

static struct platform_device m_led_dev = {
	.name                      =	"m_led",
	.id                             =	-1,
	.num_resources  =	ARRAY_SIZE(m_led_rsc),
	.resource               =	m_led_rsc,
	.dev                         =	{
						.release  = m_led_release,
					}
};
static int __init m_led_dev_init(void)
{
	platform_device_register(&m_led_dev);
	return 0;
}

static void __exit m_led_dev_exit(void)
{
	platform_device_unregister(&m_led_dev);
}

module_init(m_led_dev_init);
module_exit(m_led_dev_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("moon.cheng.2014@gmail.com");

