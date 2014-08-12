KO_NAME=led_drv
PWD:=$(shell pwd)
KERNEL_BUILD_PATH=$(HOME)/work/system/linux-2.6.22.6

obj-m += $(KO_NAME).o

default:
	$(MAKE) -C $(KERNEL_BUILD_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_BUILD_PATH) M=$(PWD) clean
