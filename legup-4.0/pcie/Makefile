
TARGETS=shared_mem scheduler

all: $(TARGETS)

$(TARGETS): riffa_driver
	$(MAKE) -C $@

riffa_driver:
	$(MAKE) -C riffa_driver

clean:
	$(MAKE) -C riffa_driver $@
	# $(MAKE) -C demo $@
	# $(MAKE) -C driver $@
	$(MAKE) -C shared_mem $@
	$(MAKE) -C scheduler $@

.PHONY: all demo driver shared_mem clean riffa_driver 
