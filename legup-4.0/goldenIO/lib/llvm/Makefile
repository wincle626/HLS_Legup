DIRS = liblegup
TARGETS = all clean

$(TARGETS):
	for dir in $(DIRS); do make $@ -C $$dir; done

.PHONY : $(TARGETS)
