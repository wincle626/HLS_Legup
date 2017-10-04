DIRS = add atomic boxfilter cordic critical critical_atomic dotproduct histogram los mandelbrot matrixmult matrixmult_pipeline matrixtrans perfecthash reduction test

include ../../Makefile.config

$(TARGETS):
	for dir in $(DIRS); do make $@ -C $$dir; done

backup:
	mkdir -p backup
	for dir in $(DIRS); do cp $$dir/*.v backup; done

.PHONY : backup
