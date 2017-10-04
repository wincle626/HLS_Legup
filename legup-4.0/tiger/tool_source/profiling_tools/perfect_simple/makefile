CFLAGS = -O -m32

.cc.o:
	gcc $(CFLAGS) -c -m32 $<

O = lookupa.o recycle.o perfhex.o perfect.o

const64 : $(O)
	gcc -m32 -o perfect $(O) -lm

# DEPENDENCIES

lookupa.o : lookupa.c standard.h lookupa.h

recycle.o : recycle.c standard.h recycle.h

perfhex.o : perfhex.c standard.h lookupa.h recycle.h perfect.h

perfect.o : perfect.c standard.h lookupa.h recycle.h perfect.h

clean: 
	rm -f *.o perfect
