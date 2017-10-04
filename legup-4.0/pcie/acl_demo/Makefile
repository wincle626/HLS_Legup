include ../../examples/Makefile.config

TARGETS = demo user reprogram

CFLAGS = -g -Wall -O3 -I ../acl_driver -DLINUX
CC = $(FRONT_END)

all: $(TARGETS)

demo: demo.o alt_up_pci_lib_helper.o
	$(CC) $(PCIE_LDFLAG) $^ -o $@

user: user.o
	$(CC) $(PCIE_LDFLAG) $^ -o $@

reprogram: reprogram.o
	$(CC) $(PCIE_LDFLAG) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGETS) *.o

.PHONY: all clean
