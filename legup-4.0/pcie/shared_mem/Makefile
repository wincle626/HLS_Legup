INC=legup_mem.h
SW_SRC=legup_mem_sw.c legup_mem_sw_test.c
SW_DBG_SRC=legup_mem_sw_debug.c legup_mem_shared.c legup_mem_shared.h legup_mem_shared_test.c
SHARED_SRC=legup_mem.c legup_mem_shared.c legup_mem_shared_test.c

TARGET_SRC=legup_mem.c legup_mem_shared.c

TESTS=sw_test sw_debug_test shared_test

TARGET=libLegUpSharedMem.a
TARGET_OBJS=legup_mem.o legup_mem_shared.o
PWD=`pwd`

all: $(TESTS) $(TARGET)
	ln -fs $(PWD)/$(TARGET) ../lib/$(TARGET)

%.o: %.c
	gcc -g -fPIC -I ../riffa_driver -L ../riffa_driver -lriffa -c $< -o $@

sw_test: $(INC) $(SW_SRC)
	gcc -g $^ -o $@

sw_debug_test: $(INC) $(SW_DBG_SRC)
	gcc -g $^ -o $@

shared_test: $(INC) $(SHARED_SRC)
	gcc -g -I ../riffa_driver $^ -o $@ -L ../lib -lriffa

$(TARGET): $(TARGET_OBJS)
	ar rcs $(TARGET) $(TARGET_OBJS)

run_test:
	./sw_test
	./sw_debug_test
	./shared_test

clean:
	rm -f $(TESTS) $(TARGET) $(TARGET_OBJS) ../lib/$(TARGET)

.PHONY: all clean
