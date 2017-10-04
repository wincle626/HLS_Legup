# This is a special makefile format for compiling kernel drivers
ifeq ($(shell uname -m),ppc64)
EXTRA_CFLAGS += -DACLPCI_BIGENDIAN
endif

ifneq ($(KERNELRELEASE),)
	obj-m := aclpci_drv.o
	aclpci_drv-y := aclpci_queue.o aclpci.o aclpci_fileio.o aclpci_dma.o aclpci_cvp.o aclpci_cmd.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

clean-files := *.o *.ko *.mod.[co] *~
