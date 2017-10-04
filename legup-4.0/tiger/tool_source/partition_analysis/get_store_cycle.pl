# Lanny: July-20-2012
#!/usr/bin/perl
use warnings;
use strict;

(defined $ARGV[0]) or die "Expect accelerator name as argument 0.";
(defined $ARGV[1]) or die "Expect the filtered store trace file as argument 1.";
(defined $ARGV[2]) or die "Expect the filename of output report as argument 2.\n";

my $l = "";
my $func_name = $ARGV[0];
my $boundary = hex("0x90000000"); # any addr greater than this value is considered as stack

# generating trace of load address of accelerating this function
my $in_func = 0;      # indicates if current line is inside accelerating function
my $num_calls = 0;    # number of calls to accelerating function
my $verify_depth = 0; # used to verify if the program is properly traced; expected to be 0 at the end of trace
my $depth = 0;        # function calling depth inside accelerating function
my $sp = 0;           # sp right before calling accelerating function
my $global_sw_cntr = 0;
my $local_sw_cntr = 0;

open SRC, $ARGV[1] or die $!;
while (<SRC>) {
	chop;
	my $l = $_;
	if ($in_func == 0) { # outside of the accelerating function
		if ($l =~ /^<calling /) {			
			if ($l =~ /^<calling $func_name>/) {
				$in_func = 1;
				$num_calls += 1;
				$depth = 0;
				$sp = $l;
				$sp =~ s/.*\> sp-\>//;
				$sp = hex($sp);
			}
			else {
				$verify_depth += 1;
			}
		}
		elsif ($l eq "<returning>") {
			$verify_depth -= 1
		}
	}
	else { # inside of the accelerating function
		if ($l =~ /^<calling [^\s]*>/) {
			($l !~ /^\<calling $func_name\>/) or die "Error: the accelerating function is calling itself...\n";
			$depth += 1;
		}
		elsif ($l eq "<returning>") {
			$depth -= 1;
			if ($depth==-1) {
				$in_func = 0;
			}
		}
		else {
			if (hex($l)<$boundary or hex($l)>=$sp){ # global variable but stack belong to parent functions (parents run on MIPS)
				$global_sw_cntr += 1;
			}
			else { # on local stack
				$local_sw_cntr += 1;
			}
		}		
	}
}
close SRC;

# check if the trace is well parsed
$depth == -1 or die "Error: the program is not properly traced. depth=$depth\n";
$verify_depth == -1 or die "Error: the program is not properly traced. verify_depth=$verify_depth\n";

system "echo Function: $func_name \> $ARGV[2]";
system "echo $global_sw_cntr stores to global memory space. \>\> $ARGV[2]";
system "echo $local_sw_cntr stores to local memory space. \>\> $ARGV[2]";

