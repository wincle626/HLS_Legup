ifneq ($(KERNELRELEASE),)
	obj-m	+= alt_up_pci.o
	alt_up_pci-objs := alt_up_pci_driver.o alt_up_pci_sdma.o alt_up_pci_sgdma.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

clean-files := *.o *.ko *.mod.[co] *~
