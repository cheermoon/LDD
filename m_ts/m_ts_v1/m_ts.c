#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>  /*input device subsystem*/
#include <linux/clk.h>	/*clock*/
#include <linux/irq.h> 	/*interrupt*/
#include <linux/interrupt.h>
#include <asm/io.h>	
 
#define DRV_VERSION "v 0.1"

struct ts_regs {
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
};

static struct input_dev *m_ts_input_dev;
static volatile struct ts_regs *m_ts_regs;

static void m_adc_start(void)
{
	unsigned long tmp;
	tmp = ioread32(&m_ts_regs->adccon);
	tmp |= 1;
	iowrite32(tmp, &m_ts_regs->adccon);
}

static void m_measure_xy_mode(void)
{
	unsigned long tmp;
	tmp = ioread32(&m_ts_regs->adctsc);
	tmp |= (3<<2);
	iowrite32(tmp, &m_ts_regs->adctsc);
}
static void touch_down_detect_mode(void)
{
	iowrite16(0xd3, &(m_ts_regs->adctsc));	
}

static void touch_up_detect_mode(void)
{
	iowrite16(0x1d3, &(m_ts_regs->adctsc));
}
static irqreturn_t m_touch_irq_handle(int irq, void *dev)
{
	if (ioread16(&(m_ts_regs->adcdat0)) & (1<<15)) { //touch up
		printk(KERN_INFO"Touch up\n");
		touch_down_detect_mode();
	} else {//touch down
		
		m_measure_xy_mode();
		m_adc_start();
		 /*
		printk(KERN_INFO"Touch down\n");
		touch_up_detect_mode();
		*/
	}
	return IRQ_HANDLED;
}

static irqreturn_t m_adc_irq_handle(int irq, void *dev)
{
	unsigned long adcdat0;
	unsigned long adcdat1;

	adcdat0 = m_ts_regs->adcdat0;
	adcdat1 = m_ts_regs->adcdat1;

	if (adcdat0 & (1 << 15))
	{
		printk(KERN_INFO"Touch up\n");
		touch_down_detect_mode();
	} else {
		printk(KERN_INFO"x= %ld y=%ld\n", adcdat0 & 0x3ff, adcdat1 & 0x3ff);
		touch_up_detect_mode();
	}
	
	return IRQ_HANDLED;
}
static int m_ts_drv_init(void)
{
	int ret;
	struct clk *clk;

	/*1. Apply a input device & config it & register it*/
	m_ts_input_dev = input_allocate_device();
	/*set events*/
	set_bit(EV_ABS, m_ts_input_dev->evbit);
	set_bit(EV_KEY, m_ts_input_dev->evbit);
	/*set codes*/
	set_bit(BTN_TOUCH, m_ts_input_dev->keybit);
	/*set input parameters*/
	input_set_abs_params(m_ts_input_dev, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(m_ts_input_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(m_ts_input_dev, ABS_PRESSURE, 0, 1, 0, 0);
	/*register input device*/
	input_register_device(m_ts_input_dev);
	
	/*2.Init touch screen hardware*/
	/*get&enable clk*/
	clk = clk_get(NULL, "adc");
	clk_enable(clk);
	/*remap the regs*/
	m_ts_regs = ioremap(0x5800000, sizeof(struct ts_regs));
	iowrite32((1<<14  | 49<<6), &(m_ts_regs->adccon));
	/*request IRQ*/
	ret = request_irq(IRQ_TC,  m_touch_irq_handle, IRQF_SAMPLE_RANDOM,	"m_ts", NULL);
	ret = request_irq(IRQ_ADC, m_adc_irq_handle, IRQF_SAMPLE_RANDOM, "m_adc", NULL);
	
	return 0;
}

static void m_ts_drv_exit(void)
{
	input_unregister_device(m_ts_input_dev);
	input_free_device(m_ts_input_dev);
	free_irq(IRQ_TC, NULL);
	free_irq(IRQ_ADC, NULL);
}

module_init(m_ts_drv_init);
module_exit(m_ts_drv_exit);

MODULE_AUTHOR("moon.cheng.2014@gmail.com");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION("this is a touch screen test driver created by moon.cheng");
