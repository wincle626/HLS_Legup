DIRS = adpcm aes blowfish dfadd dfdiv dfmul dfsin gsm jpeg mips motion sha dhrystone

include ../Makefile.config

$(TARGETS):
	for dir in $(DIRS); do make $@ -C $$dir; done
