# Lanny: June-28-2012
#!/usr/bin/perl
use warnings;
use strict;
use Cwd;

defined $ARGV[0] or die "Error: expect the list of functions to be predicted as argument 0.\n";

open TARGET, $ARGV[0];
my @targets=<TARGET>;
close TARGET;

foreach my $target (@targets) {
	chomp ($target);
	if ($target =~ /^\s*#/) {next;}

	open CONFIG, ">config.tcl";
	print CONFIG "set_accelerator_function \"$target\"\n";
	close CONFIG;
	
	# RUN
	system ("make predictHybridCycle ACCELERATOR_NAME=$target");
}
