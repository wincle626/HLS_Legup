#!/usr/bin/perl 
#
# USAGE: genRAM.pl <family> <file>
# This script parses the config.tcl file for specified cache parameters, generates the altsyncRAMs accordingly, and also changes cache_parameters.v with the new values

use warnings;
use strict;

# FPGA family (Stratix IV or Cyclone II)
my $family = $ARGV[0];
my $filename = $ARGV[1];

parse($filename, $family, "dcache");
parse($filename, $family, "icache");

sub parse {
    my $filename = shift;
	my $family = shift;
	my $cachetype = shift;

	my $cachesize = '0'; #cache size
	my $linesize = '0'; #cache line size
	my $numcacheline = '0'; #number of cache lines
	my $way = '0'; #associativity
	my $filedata;

	my $defined = '0';

	if (-e $filename) {
		#read in file
		$filedata = qx/cat $filename/;

		#parse cache size
		if ($filedata =~ /\n*\s*set_${cachetype}_size (\d*.*\d+)/) {
			 $cachesize = $1*'1024'*'8'; #read in as KB, converted to bits
			 $defined = '1';
		}
		#check if cache size is a power of 2
		if ($cachesize != '0') {
			if (!powerof2($cachesize)) {
				die("${cachetype} size needs to be defined as a power of 2 when converted to bits!\n");
			}
		}

		#parse cache line size
		if ($filedata =~ /\n*\s*set_${cachetype}_linesize (\d+)/) {
	         $linesize = $1*8; #read in as bytes, converted to bits
			 $defined = '1';
		} 
	
		#check if line size is a power of 2
		if ($linesize != '0') {
			if (!powerof2($linesize)) {
				die("${cachetype} line size needs to be defined as a power of 2 when converted to bits!\n");
			}
			#check minimum line sizes
			if ($family eq "Stratix IV") {
				if ($linesize < 256) {
					die("minimum ${cachetype} line size for ${family} needs to be 256 bits!\n");
				}
			} else {
				if ($linesize < 32) {
					die("minimum ${cachetype} line size for ${family} needs to be 32 bits!\n");
				}
			}
		} 

		#parse cache associativity
		if ($filedata =~ /\n*\s*set_${cachetype}_way (\d+)/) {
	         $way = $1;
			 $defined = '1';
		} 		
		#check if line size is a power of 2
		if ($way != '0') {
			if (!powerof2($way)) {
				die ("${cachetype} associativity needs to be defined as a power of 2!\n");
			}
		}
		
		#print "${cachetype} size = $cachesize bits\n";
		#print "${cachetype} linesize = $linesize bits\n";
		#print "${cachetype} way = $way\n";

		#if none of the parameters are defined, then just keep default values and don't generate RAM.
		#default values are, DE4 : num cache lines = 128, line size = 1024 bits, direct-mapped. 
		#					 DE2 : num cache lines = 128, line size = 256 bits, direct-mapped.
		#if only some of the parameters are undefined, raise error. 
		if ($defined == '1') {
			if ($cachesize == '0' || $linesize == '0' || $way == '0') {
				die ("If you defined any of the parameters for ${cachetype}, then you need to do all of set_${cachetype}_size, set_${cachetype}_linesize, and set_${cachetype}_way!\n");
			} 
			#cache size needs to be converted to number of cache lines, as this is what is used in megawizard
			$numcacheline = $cachesize/($linesize*$way);
			#check if number of cache lines is a power of 2
			if (!powerof2($numcacheline)) {
				die("${cachetype} number of lines needs to be defined as a power of 2!\n");
			}
			if ($numcacheline < 1) {
				die("${cachetype} number of lines cannot be less than 1!\n");
			}
		}

		#if cache parameters are defined, run megawizard and replace cache parameters defined in cache_parameters.v
		if ($defined == '1') {
			runMegawizard($family, $cachetype, $linesize, $numcacheline);
			replaceParameters($family, $cachetype, $linesize, $numcacheline, $way);
		}

		#print cache sizes
		if ($defined == '1') {
			print "${cachetype} parameters specified.\n";
			print "${cachetype} size = $cachesize bits\n";
			print "${cachetype} number of cache lines = $numcacheline lines\n";
			print "${cachetype} linesize = $linesize bits\n";
			print "${cachetype} way = $way\n\n";
		} else {
			print "${cachetype} parameters not specified. Using default values.\n";
		}
	} 
}

sub replaceParameters {
	my $family = shift;
	my $cachetype = shift;
	my $linesize = shift;
	my $numcacheline = shift;
	my $way = shift;

	my $paramfilename = "tiger/cache_parameters.v";	

	my $cachesize = log2($numcacheline); #log2 of number of lines
	$linesize = log2($linesize/8); #log2 of line size in bytes

	if (-e $paramfilename) {
		if ($cachetype eq "dcache") {
			system("sed -i -r 's|\\s*`define\\s+DCACHE_SIZE\\s+[0-9]+|`define DCACHE_SIZE ${cachesize}|g' $paramfilename");
			system("sed -i -r 's|\\s*`define\\s+DBLOCKSIZE\\s+[0-9]+|`define DBLOCKSIZE ${linesize}|g' $paramfilename");
			system("sed -i -r 's|\\s*`define\\s+DWAYS\\s+[0-9]+|`define DWAYS ${way}|g' $paramfilename");
		} else {
			system("sed -i -r 's|\\s*`define\\s+ICACHE_SIZE\\s+[0-9]+|`define ICACHE_SIZE ${cachesize}|g' $paramfilename");
			system("sed -i -r 's|\\s*`define\\s+IBLOCKSIZE\\s+[0-9]+|`define IBLOCKSIZE ${linesize}|g' $paramfilename");
			system("sed -i -r 's|\\s*`define\\s+IWAYS\\s+[0-9]+|`define IWAYS ${way}|g' $paramfilename");
		}
	} else {
		die ("$paramfilename not found!\n");
	}
}

sub runMegawizard {
	my $family = shift;
	my $cachetype = shift;
	my $linesize = shift; #cache line size
	my $numcacheline = shift; #number of cache lines

	my $validbit = '1';
	my $tagbits;
	my $status;

	if ($cachetype eq "dcache") {
		$tagbits = '31'; #for data cache tag bits are fixed to 31 bits for byteenables
	} else {
		$tagbits = 32 - log2($numcacheline) - log2($linesize/8);
	}

	$linesize += $tagbits + $validbit; # adding in the tag bits and the valid bit,

	my $address_width = log2($numcacheline); #get the width of the address  

	#for data cache
	if ($cachetype eq "dcache") {
		system("rm -f tiger/dcacheMem.v"); #first delete existing file
		#generate new dcacheMem.v (dual-port)
		system("qmegawiz -silent module=altsyncram wizard=\"RAM: 2-PORT\" OPERATION_MODE=BIDIR_DUAL_PORT intended_device_family=\"${family}\" WIDTH_A=${linesize} WIDTH_B=${linesize} NUMWORDS_A=${numcacheline} NUMWORDS_B=${numcacheline} WIDTHAD_A=${address_width} WIDTHAD_B=${address_width} CLOCK_ENABLE_INPUT_A=BYPASS CLOCK_ENABLE_OUTPUT_A=BYPASS CLOCK_ENABLE_INPUT_B=BYPASS CLOCK_ENABLE_OUTPUT_B=BYPASS BYTE_SIZE=8 INDATA_REG_B=CLOCK0 OUTDATA_ACLR_A=NONE OUTDATA_ACLR_B=NONE OUTDATA_REG_A=UNREGISTERED OUTDATA_REG_B=UNREGISTERED POWER_UP_UNINITIALIZED=FALSE READ_DURING_WRITE_MODE_MIXED_PORTS=OLD_DATA READ_DURING_WRITE_MODE_PORT_A=NEW_DATA_NO_NBE_READ READ_DURING_WRITE_MODE_PORT_B=NEW_DATA_NO_NBE_READ WIDTH_BYTEENA_A=132 WIDTH_BYTEENA_B=132 WRCONTROL_WRADDRESS_REG_B=CLOCK0 ADDRESS_REG_B=CLOCK0 BYTEENA_REG_B=CLOCK0 LOW_POWER_MODE=NONE address_b=used data_b=used byteena_b=used data_b=used wre_b=used clocken2=unused clock1=unused clocken1=unused byteena_a=used aclr0=unused q_b=used rden_a=unused rden_b=unused aclr1=unused clocken0=unused OPTIONAL_FILES=-BB tiger/dcacheMem.v");
	} else { #for instruction cache
		system("rm -f tiger/icacheMem.v");
		#generate new icacheMem.v (single-port)
		system("qmegawiz -silent module=altsyncram wizard=\"RAM: 1-PORT\" OPERATION_MODE=SINGLE_PORT intended_device_family=\"${family}\" WIDTH_A=${linesize} NUMWORDS_A=${numcacheline} WIDTHAD_A=${address_width} CLOCK_ENABLE_INPUT_A=NORMAL CLOCK_ENABLE_OUTPUT_A=BYPASS OUTDATA_ACLR_A=NONE OUTDATA_REG_A=UNREGISTERED POWER_UP_UNINITIALIZED=FALSE READ_DURING_WRITE_MODE_PORT_A=NEW_DATA_NO_NBE_READ WIDTH_BYTEENA_A=1 LOW_POWER_MODE=NONE address_b=unused data_b=unused byteena_b=unused data_b=unused wre_b=unused clocken2=unused clock1=unused clocken1=unused byteena_a=unused aclr0=unused q_b=unused rden_b=unused aclr1=unused CLOCK_ENABLE_OUTPUT_B=BYPASS clocken0=used OPTIONAL_FILES=-BB tiger/icacheMem.v");
	}
}

sub log2 {
	my $value = shift;
	log($value)/log(2);
}
sub powerof2 { 
	not ($_[0] & $_[0]-1); 
}

