use strict;
use warnings;

package attribute;

sub new {	
	my $class = shift;
	my $self  = bless {}, $class;

	my %args  = @_;
	$self->{_attribute_name} 	= $args{attribute_name};
	$self->{_start} 			= $args{start};
	$self->{_increment} 		= $args{increment};
	$self->{_end} 				= $args{end};
	$self->{processed}			= 0;
	$self->{currentVal}			= $args{start};;
	# do something with arguments to new()
	return $self;
}

sub getAttributeName {
	my $self = shift;
	return $self->{_attribute_name};
}

sub getStartVal {
	my $self = shift;
	return $self->{_start};
}

sub getIncrementVal {
	my $self = shift;
	return $self->{_increment};
}

sub getEndVal {
	my $self = shift;
	return $self->{_end};
}
sub getCurrentVal {
	my $self = shift;
	return $self->{currentVal};
}


1
