use strict;
use warnings;

package util;

sub isBool {
	
	my $class = shift;
	my %args  = @_;
	my $attribute_name = $args{attribute_name};
	my $matched=0;
	
    my @listBoolAttributes=("ARRAY_INPUT", "FIX_BLOCK_SIZE");
    #my @matches = grep { $attribute_name } @listBoolAttributes;
    
	if (grep {$_ eq $attribute_name} @listBoolAttributes) {
	   return 1;
	}
	else {
	   return 0;
	}
}
1;