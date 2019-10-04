ifeq ($(KERNELRELEASE),)

KERNELDIR = ~/sources/rpi-4.19/
PWD := $(shell pwd)

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- -C $(KERNELDIR) M=$(PWD) modules
	scp swread.ko root@10.9.8.2:

clean:
	rm -rf *.cmd *.symvers *.order *.ko *.o *.mod.c .swread.ko.cmd .swread.mod.o.cmd .swread.o.cmd .tmp_versions .swread.o.d

else
	obj-m := swread.o
	ccflags-y := -std=gnu99 -Wno-declaration-after-statement -Werror
endif
