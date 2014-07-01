KO_NAME=module_ts
PWD:=$(shell pwd)
KERNEL_BUILD_PATH=/usr/src/linux-headers-2.6.32-38-generic

obj-m += $(KO_NAME).o

default:
	$(MAKE) -C $(KERNEL_BUILD_PATH) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_BUILD_PATH) M=$(PWD) clean
