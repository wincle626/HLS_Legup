# Lanny: June-28-2012
#!/usr/bin/perl
use warnings;
use strict;

(defined $ARGV[0]) or die "Error: Program Name is expected as argument 0.\n";
( (-e $ARGV[0].".sim.profiling.rpt") or (-e $ARGV[0].".profiling.rpt") ) or die "Error: profiling report file not found.\n";

(defined $ARGV[1]) or die "Error: Expect minimum percentage to predict as argument 1.\n";


my @skipping_func_names = (
	"main",
	"legup_memset_4",
	"legup_memset_2"
);

open OUT, ">".$ARGV[0].".predict.flist";
if (-e $ARGV[0].".profiling.rpt") {
	my $rpt = $ARGV[0].".profiling.rpt";
	my @funcs = `grep % $rpt`;
	foreach my $func (@funcs) {
		chomp $func;
		my @data = split(/\s*\|\s*/, $func);
		
		my $func_name = $data[1];
		if ( grep {$_ eq $func_name} @skipping_func_names ) {
			next;
		}

		my $value = $data[5];
		$value =~ s/.* //;
		$value =~ s/.*<//;
		$value =~ s/%.*//;
		if ( int($value) > int($ARGV[1]) ) { #hierarchical percentage of cycles > x%
			print OUT $func_name."\n";
		}
	}
}
else {
	my $rpt = $ARGV[0].".sim.profiling.rpt";
	my @funcs = `grep Percentage $rpt`;
	foreach my $func (@funcs) {
		chomp $func;
		my $func_name = $func;
		$func_name =~ s/.*, Function <//;
		$func_name =~ s/>.*//;
		if ( grep {$_ eq $func_name} @skipping_func_names ) {
			next;
		}

		my $value = $func;
		$value =~ s/.* Percentage *//;
		$value =~ s/%.*//;
		$value =~ s/.*<//;
		if ( int($value) > int($ARGV[1]) ) { #hierarchical percentage of cycles > x%
			print OUT $func_name."\n";
		}
	}

}


close OUT;
