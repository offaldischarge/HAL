ifeq ($(KERintNELRELEASE),)
#remember a line after ifeq
KERNELDIR = ~/sources/rpi-4.19/
PWD := $(shell pwd)

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- -C $(KERNELDIR) M=$(PWD) modules

else
	obj-m := helloworldmodule.o
	ccflags-y := -std=gnu99 -Wno-declaration-after-statement -Werror
endif
