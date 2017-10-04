include ../../examples/Makefile.config

TARGET = demo reprogram

CFLAGS = -g -Wall -O3
CC = gcc #$(FRONT_END)

build: $(TARGET)

demo: demo.o alt_up_pci_lib.o alt_up_pci_lib_helper.o
	$(CC) $(PCIE_LDFLAG) $^ -o $@

reprogram: reprogram.c alt_up_pci_lib.o
	cc reprogram.c alt_up_pci_lib.o -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~
