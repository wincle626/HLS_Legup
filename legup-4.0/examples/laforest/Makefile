DIRS = array_reverse_legup_3step       fir_legup_buffer   fp_fsm_legup_structured      hailstone_legup_modified \
array_increment_legup_counted  array_reverse_legup_4step_1mem  fir_legup_pointer  hailstone_legup                                                         hailstone_legup_modified2 \
array_reverse_legup_4step_2mem  fp_fsm_legup_asm   hailstone_legup_accelerated


include ../Makefile.config

$(TARGETS):
	for dir in $(DIRS); do make $@ -C $$dir; done

backup:
	mkdir -p backup
	for dir in $(DIRS); do cp $$dir/*.v backup; done

.PHONY : backup

tarball:
	rm data.tar
	tar cvf data.tar */fir_legup.c */hailstone_legup.c */array_increment_legup.c */array_reverse_legup.c */fp_fsm_legup_asm.c */top.fit.rpt */top.sta.rpt
