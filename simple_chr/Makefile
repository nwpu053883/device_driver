ifneq ($(KERNELRELEASE), )
	obj-m += simple_chr.o
else
KERN_VERS=$(shell uname -r)
KERN_DIR=/lib/modules/$(KERN_VERS)/build

new:drivers

drivers:
	make -C $(KERN_DIR) M=$(PWD) modules

clean:
	make -C $(KERN_DIR) M=$(PWD) clean

endif
