DIRS = add alphablend barrier blackscholes blackscholes_LUT boxfilter cordic df dfsin dividers dividers_pipeline divstore divstore_pipeline dotproduct hash hash_mod hash_mod_pipeline histogram los los2 mandelbrot mandelbrot_pipeline matrixmult matrixmult_pipeline matrixtrans mcml_simple perfecthash primestore primestore_pipeline

include ../../Makefile.config

$(TARGETS):
	for dir in $(DIRS); do make $@ -C $$dir; done

backup:
	mkdir -p backup
	for dir in $(DIRS); do cp $$dir/*.v backup; done

.PHONY : backup
