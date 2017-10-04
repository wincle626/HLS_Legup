DIRS = divstore mandelbrot primestore hash1k los2 dfsin

include ../../Makefile.config

$(TARGETS):
	for dir in $(DIRS); do make $@ -C $$dir; done
