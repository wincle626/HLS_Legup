
TARGET=reprog pci_read_direct pci_write_direct small large speed speed_multithread speed_1M_1000 speed_10M_100 small_write read write_test mis-aligned-read

CFLAGS=-g --std=c99 -O0 -I ../riffa_driver
LDFLAGS=-L ../lib -lriffa -lm -lpthread

all: $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

reprog: reprog.o
	$(CC) $< -o $@ $(LDFLAGS)

pci_write_direct: pci_write_direct.o
	$(CC) $< -o $@ $(LDFLAGS)

pci_read_direct: pci_read_direct.o
	$(CC) $< -o $@ $(LDFLAGS)

small: small.o
	$(CC) $< -o $@ $(LDFLAGS)

large: large.o
	$(CC) $< -o $@ $(LDFLAGS)

speed: speed.o
	$(CC) $< -o $@ $(LDFLAGS)

speed_1M_1000: speed_1M_1000.o
	$(CC) $< -o $@ $(LDFLAGS)

speed_10M_100: speed_10M_100.o
	$(CC) $< -o $@ $(LDFLAGS)

speed_multithread: speed_multithread.o
	$(CC) $< -o $@ $(LDFLAGS)

small_write: small_write.o
	$(CC) $< -o $@ $(LDFLAGS)

read: read.o
	$(CC) $< -o $@ $(LDFLAGS)

mis-aligned-read: mis-aligned-read.o
	$(CC) $< -o $@ $(LDFLAGS)

write_test: write_test.o
	$(CC) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o
