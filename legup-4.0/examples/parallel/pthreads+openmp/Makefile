DIRS = divstore_2 mandelbrot_2 primestore_2 hash1k_2 los2_2 dfsin_2 divstore_3 mandelbrot_3 primestore_3 hash1k_3 los2_3 dfsin_3 divstore_4 mandelbrot_4 primestore_4 hash1k_4 los2_4 dfsin_4

include ../../Makefile.config

$(TARGETS):
	for dir in $(DIRS); do make $@ -C $$dir; done
