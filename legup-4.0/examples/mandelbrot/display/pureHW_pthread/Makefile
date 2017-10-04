NAME=mandelbrot
CUSTOM_VERILOG = 1
LOCAL_CONFIG = -legup-config=config.tcl
LEVEL = ../../..
include $(LEVEL)/Makefile.common
UNROLL = -disable-loop-unrolling

clean_mandelbrot:
	rm -rf ./proj_files/db ./proj_files/incremental_db ./proj_files/hc_output
	find ./proj_files ! -name pureHW_VGA.v ! -name pureHW_VGA.qsf ! -name pureHW_VGA.sof ! -name pureHW_VGA.cdf ! -name pureHW_VGA.qpf -type f -delete

compile_mandelbrot:
	cp *.mif ./proj_files/
	quartus_sh --flow compile ./proj_files/pureHW_VGA

program_mandelbrot:
	quartus_pgm -c DE-SoC -m jtag proj_files/pureHW_VGA.cdf

go_mandelbrot:
	make
	make compile_mandelbrot
	make program_mandelbrot

