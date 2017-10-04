# Lanny: July-20-2012
#!/usr/bin/perl
use warnings;
use strict;

(defined $ARGV[0]) or die "Expect accelerator name as argument 0.";
(defined $ARGV[1] and (-e $ARGV[1])) or die "Expect the converted trace file as argument 1.";
(defined $ARGV[2]) or die "Expect the output file of load address as argument 2.";

my $l = "";
my $func_name = $ARGV[0];
my $boundary = hex("0x40f00000"); # any addr greater than this value is considered as stack

# generating trace of load address of accelerating this function
my $in_func = 0;      # indicates if current line is inside accelerating function
my $num_calls = 0;    # number of calls to accelerating function
my $verify_depth = 0; # used to verify if the program is properly traced; expected to be 0 at the end of trace
my $depth = 0;        # function calling depth inside accelerating function
my $sp = 0;           # sp right before calling accelerating function
	
open SRC, $ARGV[1] or die $!;
open DST, ">".$ARGV[2] or die $!;
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
				print DST "<Accelerator Started>\n";
			}
			else {
				$verify_depth += 1;
			}
		}
		elsif ($l eq "<returning>") {
			$verify_depth -= 1
		}
		else {
			$l =~ s/ .*//;
			print DST $l."\n";
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
				print DST "<Accelerator Finished>\n";
			}
		}
		else {
			if ($l =~ / <CONST>/) { # global constant - eliminating
				$l =~ s/ .*//;
				( hex($l) < $boundary) or die "Error: Global Constant($l) can't be greater than boundary - $boundary.\n";
			}
			elsif (hex($l) < $boundary) { # global variable
				print DST $l."\n";
			}
			else { # on stack
				if ( hex($l) >= $sp ) { # not belong to local memory space of accelerating function
					print DST $l."\n";
				}
			}
		}		
	}
}
close DST;
close SRC;

# check if the trace is well parsed
$verify_depth == -1 or die "Error: the program is not properly traced. verify_depth=$verify_depth\n";

