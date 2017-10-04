
INC=LegUpScheduler.h
SRC=LegUpScheduler.c
OBJS=LegUpScheduler.o

CC=gcc
PWD=`pwd`

TARGET=libLegUpScheduler.a

all: $(TARGET)
	ln -fs $(PWD)/$(TARGET) ../lib/$(TARGET)

TestMain: $(TARGET) main.o
	gcc -g main.o -L . -lLegUpScheduler -lpthread -o TestMain 

$(TARGET): $(OBJS)
	ar rcs $(TARGET) $(OBJS)

%.o: %.c
	$(CC) -g -fPIC -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) ../lib/$(TARGET)

.PHONY: all clean
