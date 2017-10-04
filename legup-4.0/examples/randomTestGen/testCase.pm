use strict;
use warnings;

package testCase;

sub new {
	my $class = shift;
	my $self  = bless {}, $class;
	my %args  = @_;
	$self->{_test_name} = $args{test_name};
	$self->{attributeList} = undef;
	# do something with arguments to new()
	return $self;
}
sub run_test{
	my $self = shift;
	my @attributeList_obj=@{$self->{attributeList}};
	print "Executing test case: $self->{_test_name}\n";
	
	open FILE, ">autoGen.conf" or die $!;
	
	for my $attri (@attributeList_obj) {
        my $atti_name=$attri->getAttributeName();
        my $atti_val =$attri->getCurrentVal();
	    print "The Attribute name is $atti_name: $atti_val\n";
	    print FILE "$atti_name: $atti_val\n";
    }
}
sub outputConfigFile {
   my $self = shift;
}
1;