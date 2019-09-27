ifeq ($(KERNELRELEASE),)

KERNELDIR = ~/sources/rpi-4.19/
PWD := $(shell pwd)

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- -C $(KERNELDIR) M=$(PWD) modules
	scp ledread.ko root@10.9.8.2:

clean:
	rm -rf *.cmd *.symvers *.order *.ko *.o *.mod.c .ledread.ko.cmd .ledread.mod.o.cmd .ledread.o.cmd .tmp_versions

else
	obj-m := ledread.o
	ccflags-y := -std=gnu99 -Wno-declaration-after-statement -Werror
endif
