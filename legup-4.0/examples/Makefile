BENCHMARK = dhrystone chstone
BENCHMARK_FOLDERS = chstone/jpeg chstone/adpcm chstone/blowfish chstone/dfmul chstone/dfdiv chstone/dfadd chstone/motion chstone/sha chstone/dfsin chstone/aes chstone/mips chstone/gsm dhrystone
BENCHMARK_HYBRID_FOLDERS = chstone_hybrid/jpeg chstone_hybrid/adpcm chstone_hybrid/blowfish chstone_hybrid/dfmul chstone_hybrid/dfdiv chstone_hybrid/dfadd chstone_hybrid/motion chstone_hybrid/sha chstone_hybrid/dfsin chstone_hybrid/aes chstone_hybrid/mips chstone_hybrid/gsm chstone_hybrid/dhrystone
# there is no mips in the 2nd hybrid
BENCHMARK_FOLDERS2 = chstone/jpeg chstone/adpcm chstone/blowfish chstone/dfmul chstone/dfdiv chstone/dfadd chstone/motion chstone/sha chstone/dfsin chstone/aes chstone/gsm dhrystone
DIRS = intrinsics array cpp div_const fir function_pointer functions hierarchy_test llist loadstore long_cmp loop loopbug malloc memset phi select shift signeddiv simplestruct sra struct switches unaligned fft overflow_intrinsic memory_access_test $(BENCHMARK) $(BENCHMARK_HYBRID_FOLDERS)
LEVEL = .

include Makefile.config

$(TARGETS):
	for dir in $(DIRS); do make $@ -C $$dir; done

# include output from tests
# Surprisingly you need to manually set status=1
# to ensure runtest fails on all tcl errors
RUNTEST = runtest -v -v -v -v --all --status=1 --tool=nomatch

#set_family_DE1_SoC:
#$(eval FAMILY := CycloneV)

#set_family_DE4:
#$(eval FAMILY := StratixIV)

set_board_DE1_SoC_ARMA9:
	$(eval FAMILY := CycloneV)
	$(eval OVERRIDE_BOARD := DE1-SoC-ARMA9)

set_board_DE1_SoC:
	$(eval FAMILY := CycloneV)
	$(eval OVERRIDE_BOARD := DE1-SoC)

set_board_SoCKit_ARMA9:
	$(eval FAMILY := CycloneV)
	$(eval OVERRIDE_BOARD := SoCKit-ARMA9)

set_board_SoCKit:
	$(eval FAMILY := CycloneV)
	$(eval OVERRIDE_BOARD := SoCKit)

set_board_DE4:
	$(eval FAMILY := StratixIV)
	$(eval OVERRIDE_BOARD := DE4)


################################################################################
# Quick Sanity Check Before Commits
################################################################################

check:
	runtest ../dejagnu/check.exp




################################################################################
# Hardware-Only Tests
################################################################################

# should finish quickly (doesn't run jpeg)
test:
	$(RUNTEST) `find -name "dg.exp" | sort`

test_jpeg:
	$(RUNTEST) ../dejagnu/jpeg.exp

# Run a full Quartus compile for all the benchmark circuits
benchmark:
	for dir in $(BENCHMARK); do make p -C $$dir; done
	for dir in $(BENCHMARK); do make f -C $$dir; done
	./benchmark.pl --family $(FAMILY)


################################################################################

hw:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) `find -name "dg.exp" | sort`

hw_parallel:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hw_parallel/*.exp

# Run a full Quartus compile for all the benchmark circuits
hw_benchmark:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	for dir in $(BENCHMARK); do make p -C $$dir; done
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	for dir in $(BENCHMARK); do make f -C $$dir; done
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	./benchmark.pl --family $(FAMILY)

hw_DE1_SoC: set_board_DE1_SoC hw
hw_DE4: set_board_DE4 hw

hw_parallel_DE1_SoC: set_board_DE1_SoC hw_parallel
hw_parallel_DE4: set_board_DE4 hw_parallel

hw_benchmark_DE1_SoC: set_board_DE1_SoC hw_benchmark
hw_benchmark_DE4: set_board_DE4 hw_benchmark



################################################################################
# Software-Only Tests
################################################################################

test_tiger_sim_1:
	export OVERRIDE_BOARD=DE1-SoC;\
	$(RUNTEST) ../dejagnu/tiger_sim_1/*.exp

test_tiger_sim_2:
	export OVERRIDE_BOARD=DE1-SoC;\
	$(RUNTEST) ../dejagnu/tiger_sim_2/*.exp

test_tiger_sim_3:
	export OVERRIDE_BOARD=DE1-SoC;\
	$(RUNTEST) ../dejagnu/tiger_sim_3/*.exp

test_tiger_sim_4:
	export OVERRIDE_BOARD=DE1-SoC;\
	$(RUNTEST) ../dejagnu/tiger_sim_4/*.exp

test_tiger_sim_DE4_1:
	export OVERRIDE_BOARD=DE4;\
	$(RUNTEST) ../dejagnu/tiger_sim_1/*.exp

test_tiger_sim_DE4_2:
	export OVERRIDE_BOARD=DE4;\
	$(RUNTEST) ../dejagnu/tiger_sim_2/*.exp

test_tiger_sim_DE4_3:
	export OVERRIDE_BOARD=DE4;\
	$(RUNTEST) ../dejagnu/tiger_sim_3/*.exp

test_tiger_sim_DE4_4:
	export OVERRIDE_BOARD=DE4;\
	$(RUNTEST) ../dejagnu/tiger_sim_4/*.exp

################################################################################

sw_mipsI_chstone:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/sw_chstone/*.exp

sw_mipsI_other:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/sw_other/*.exp

sw_mipsI_profile:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/sw_profile/*.exp

sw_mipsI_chstone_DE1_SoC: set_board_DE1_SoC sw_mipsI_chstone
sw_mipsI_chstone_DE4: set_board_DE4 sw_mipsI_chstone

sw_mipsI_other_DE1_SoC: set_board_DE1_SoC sw_mipsI_other
sw_mipsI_other_DE4: set_board_DE4 sw_mipsI_other

sw_mipsI_profile_DE1_SoC: set_board_DE1_SoC sw_mipsI_profile
sw_mipsI_profile_DE4: set_board_DE4 sw_mipsI_profile


################################################################################
# Hybrid Tests
################################################################################

# simulate the hybrids
hybrid_sim_1:
	export OVERRIDE_BOARD=DE1-SoC;\
	$(RUNTEST) ../dejagnu/hybrid_sim_1/*.exp

hybrid_sim_2:
	export OVERRIDE_BOARD=DE1-SoC;\
	$(RUNTEST) ../dejagnu/hybrid_sim_2/*.exp

hybrid_sim_DE4_1:
	export OVERRIDE_BOARD=DE4;\
	$(RUNTEST) ../dejagnu/hybrid_sim_1/*.exp

hybrid_sim_DE4_2:
	export OVERRIDE_BOARD=DE4;\
	$(RUNTEST) ../dejagnu/hybrid_sim_2/*.exp

# simulate parallel benchmarks for hybrids
hybrid_parallel_sim:
	export OVERRIDE_BOARD=DE1-SoC;\
	$(RUNTEST) ../dejagnu/hybrid_parallel_sim/*.exp

hybrid_parallel_sim_DE4:
	export OVERRIDE_BOARD=DE4;\
	$(RUNTEST) ../dejagnu/hybrid_parallel_sim/*.exp

# simulate jpeg benchmark for hybrids
hybrid_jpeg:
	export OVERRIDE_BOARD=DE1-SoC;\
	$(RUNTEST) ../dejagnu/hybrid_jpeg/*.exp

hybrid_jpeg_DE4:
	export OVERRIDE_BOARD=DE4;\
	$(RUNTEST) ../dejagnu/hybrid_jpeg/*.exp

################################################################################

hybrid_mipsI_chstone:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_chstone/*.exp

hybrid_mipsI_jpeg:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_jpeg/*.exp

hybrid_mipsI_other:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_other/*.exp

hybrid_mipsI_parallel:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_parallel/*.exp

hybrid_mipsI_chstone_DE1_SoC: set_board_DE1_SoC hybrid_mipsI_chstone
hybrid_mipsI_jpeg_DE1_SoC: set_board_DE1_SoC hybrid_mipsI_jpeg
hybrid_mipsI_other_DE1_SoC: set_board_DE1_SoC hybrid_mipsI_other

hybrid_mipsI_parallel_DE1_SoC: set_board_DE1_SoC hybrid_mipsI_parallel
hybrid_mipsI_parallel_DE4: set_board_DE4 hybrid_mipsI_parallel


################################################################################
# ARM Tests
################################################################################

sw_chstone_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/sw_chstone_on_board/*.exp

sw_arma9_chstone_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 sw_chstone_on_board
sw_arma9_chstone_on_board_SoCKit: set_board_SoCKit_ARMA9 sw_chstone_on_board

sw_other_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/sw_other_on_board/*.exp

sw_arma9_other_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 sw_other_on_board
sw_arma9_other_on_board_SoCKit: set_board_SoCKit_ARMA9 sw_other_on_board

sw_chstone_qemu:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/sw_chstone_qemu/*.exp

sw_arma9_chstone_qemu: set_board_DE1_SoC_ARMA9 sw_chstone_qemu
#sw_mipsI_chstone_qemu: set_board_DE1_SoC sw_chstone_qemu

sw_other_qemu:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/sw_other_qemu/*.exp

sw_arma9_other_qemu: set_board_DE1_SoC_ARMA9 sw_other_qemu
#sw_mipsI_other_qemu: set_board_DE1_SoC sw_other_qemu


################################################################################
# Hybrid On-Board Tests
################################################################################

hybrid_chstone_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_chstone_on_board/adpcm.exp
#$(RUNTEST) ../dejagnu/hybrid_chstone_on_board/*.exp

hybrid_arma9_chstone_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 hybrid_chstone_on_board
hybrid_arma9_chstone_on_board_SoCKit: set_board_SoCKit_ARMA9 hybrid_chstone_on_board

hybrid_jpeg_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_jpeg_on_board/*.exp

hybrid_arma9_jpeg_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 hybrid_jpeg_on_board
hybrid_arma9_jpeg_on_board_SoCKit: set_board_SoCKit_ARMA9 hybrid_jpeg_on_board

hybrid_other_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_other_on_board/*.exp

hybrid_arma9_other_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 hybrid_other_on_board
hybrid_arma9_other_on_board_SoCKit: set_board_SoCKit_ARMA9 hybrid_other_on_board

hybrid_parallel_openmp_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_parallel_on_board/OpenMP/*.exp

hybrid_parallel_arma9_openmp_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 hybrid_parallel_openmp_on_board
hybrid_parallel_arma9_openmp_on_board_SoCKit: set_board_SoCKit_ARMA9 hybrid_parallel_openmp_on_board

hybrid_parallel_pthreads_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_parallel_on_board/Pthreads/*.exp

hybrid_parallel_arma9_pthreads_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 hybrid_parallel_pthreads_on_board
hybrid_parallel_arma9_pthreads_on_board_SoCKit: set_board_SoCKit_ARMA9 hybrid_parallel_pthreads_on_board

hybrid_parallel_single_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_parallel_on_board/Single/*.exp

hybrid_parallel_arma9_single_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 hybrid_parallel_single_on_board
hybrid_parallel_arma9_single_on_board_SoCKit: set_board_SoCKit_ARMA9 hybrid_parallel_single_on_board

hybrid_parallel_pthreads_openmp_on_board:
	export OVERRIDE_BOARD=$(OVERRIDE_BOARD);\
	$(RUNTEST) ../dejagnu/hybrid_parallel_on_board/Pthreads+OpenMP/*.exp

hybrid_parallel_arma9_pthreads_openmp_on_board_DE1_SoC: set_board_DE1_SoC_ARMA9 hybrid_parallel_pthreads_openmp_on_board
hybrid_parallel_arma9_pthreads_openmp_on_board_SoCKit: set_board_SoCKit_ARMA9 hybrid_parallel_pthreads_openmp_on_board

################################################################################
# Debugger Tests
################################################################################

test_debug:
	$(RUNTEST) ../dejagnu/debug/debug.exp



################################################################################
# Old targets that may not be used any more
################################################################################


# compile hybrids with quartus
hybrid_quartus:
	for dir in $(BENCHMARK_FOLDERS); do make hybridquartus -C $$dir; done

hybridtest:
	make hybrid_sim_1 | tee hybridtest.log
	make hybrid_parallel_sim | tee -a hybridtest.log

hybrid_parse:
	# these files are parsed by benchmark.pl for the buildbot plots
	for dir in $(BENCHMARK_FOLDERS); do make benchmarkfiles -C $$dir; done
	./benchmark.pl --family $(FAMILY)
	cat benchmark.csv

# compile 2nd hybrids with quartus
hybrid2_quartus:
	for dir in $(BENCHMARK_FOLDERS2); do make hybrid2quartus -C $$dir; done

# simulate the 2nd hybrids
hybrid2_sim:
	$(RUNTEST) ../dejagnu/hybrid2_sim/*.exp

hybrid2_parse:
	# these files are parsed by benchmark.pl for the buildbot plots
	for dir in $(BENCHMARK_FOLDERS2); do make benchmarkfiles2 -C $$dir; done
	./benchmark.pl --family $(FAMILY)
	cat benchmark.csv

estimate:
	./runnoop.pl

test_pcie:
	# Don't use display
	xvfb-run $(RUNTEST) ../dejagnu/pcie/*.exp

tiger_quartus:
	# compile tiger standalone processor
	cd $(PROC_DIR)/.. && quartus_sh --flow compile tiger_top

tiger_sim:
	# simulate benchmarks on tiger
	for dir in $(BENCHMARK); do make tigersim -C $$dir; done

tiger_parse:
	# these files are parsed by benchmark.pl for the buildbot plots
	# every benchmark is run on tiger - they all have the same area/fmax
	for dir in $(BENCHMARK_FOLDERS); do cp -f $(PROC_DIR)/../tiger_top.tan.summary $$dir/top.tan.summary; done
	for dir in $(BENCHMARK_FOLDERS); do cp -f $(PROC_DIR)/../tiger_top.fit.summary $$dir/top.fit.summary; done
	./benchmark.pl --family $(FAMILY)
	cat benchmark.csv

test_full:
	# run everything (including jpeg) with:
	# NO_OPT=0 NO_INLINE=1
	# NO_OPT=1 NO_INLINE=1
	$(RUNTEST) ../dejagnu/full_test/*.exp

test_legup:
	make cleanall

test_tiger_gxemul:
	make cleanall

test_tiger_gxemul_no_opt:
	make cleanall

test_legup_no_opt:
	make cleanall

test_tiger_jpeg:
	make cleanall

test_legup_jpeg:
	make cleanall


# Create a tarball of all the chstone benchmarks + dhrystone
# ie. benchmarks-Aug-18.tar.gz
folder=benchmarks-`date +%b-%d-%Y`
define TARBALL_README
Legup Benchmarks
-----------------
To simulate mips.v with modelsim:
	cd chstone/mips
	vlib work
	vlog -v ../../lib/altera_mf.v ../../lib/220model.v mips.v
	vsim -c -do "run 7000000000000000ns; exit;" work.main_tb

All benchmarks should finish with return_val = 0.
endef
export TARBALL_README
tarball:
	# for cleanup:
	for dir in $(BENCHMARK_FOLDERS); do rm $$dir/*.v ; done
	for dir in $(BENCHMARK_FOLDERS); do rm $$dir/*.mif ; done
	# generate verilog (including jpeg)
	for dir in $(BENCHMARK); do make -C $$dir; done
	# copy over verilog into temporary folder
	mkdir -p ${folder}
	mkdir -p ${folder}/lib
	for dir in $(BENCHMARK_FOLDERS); do mkdir -p ${folder}/$$dir; done
	for dir in $(BENCHMARK_FOLDERS); do cp $$dir/*.v ${folder}/$$dir; done
	for dir in $(BENCHMARK_FOLDERS); do cp $$dir/*.mif ${folder}/$$dir; done
	# include altera modules
	cp ${VERILOG_LIBS} ${folder}/lib/
	# add a readme
	echo "$$TARBALL_README" > ${folder}/README
	# tarball benchmarks
	tar cvzf ${folder}.tar.gz ${folder}

.PHONY : $(TARGETS) benchmark estimate test tarball test_tiger_sim test_full test_legup test_tiger_gxemul test_tiger_gxemul_no_opt test_legup_no_opt test_tiger_jpeg test_legup_jpeg hybrid_sim hybrid_parse hybrid_quartus tiger_parse tiger_sim tiger_quartus
