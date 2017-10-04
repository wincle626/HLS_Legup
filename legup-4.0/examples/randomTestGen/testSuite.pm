use strict;
use warnings;
#use Scalar::Util qw(looks_like_number);
use attribute;
use util;
use testCase;

package testSuite;

sub new {
	my $class = shift;
	my $self  = bless {}, $class;
	my %args  = @_;
	$self->{_test_suite_name} = $args{test_suite_name};
	$self->{_template_file}   = $args{template_file};
	$self->{attributeList} = undef;
	#$self->{attributeList_loaded} = 0;
	# do something with arguments to new()
	return $self;
}

sub getTemplateFileName {
	my $self = shift;
	return $self->{_template_file};
}

sub readTemplateFile {
	use Scalar::Util qw(looks_like_number);

	my $self = shift;
	#print $self->getTemplateFileName(), "\n";
	my $temp_file_name = $self->getTemplateFileName();
	open( TEMPLATE, $temp_file_name ) or die("Could not open the template file.");

	my $count = 0;
	my $colon = ":";
	my @attributeList;
	
	foreach my $line (<TEMPLATE>) {
		
		if (length($line)==1){
			next;
		}
		my @tokens = split(/ /, $line);

		#Handling the sweeping ranges
		if (index($tokens[1], $colon) != -1) {
			#my $sweep_data = chomp($tokens[1]);
			chomp($tokens[1]);
			my $sweep_range = $tokens[1];
			#printf "%s\n", $sweep_range;
			my @sweep_data = split(/:/, $sweep_range);
			if (!looks_like_number($sweep_data[0]) || !looks_like_number($sweep_data[1]) || !looks_like_number($sweep_data[2])){
                printf("Attribute %s has a invalid value\n", $tokens[0]);
                die();
            }
			my $CurrentAttribute = attribute->new(attribute_name=>$tokens[0], start=>$sweep_data[0], increment=>$sweep_data[1], end=>$sweep_data[2]);
			push(@attributeList, $CurrentAttribute);
			
		}else{
		#Handling the fixed values
		
            my $atti_name=$tokens[0];
            chomp($tokens[1]);
            my $atti_val =$tokens[1];
            my $CurrentAttribute;
            
            if (looks_like_number($atti_val) ) {
            	$CurrentAttribute = attribute->new(attribute_name=>$atti_name, start=>$atti_val, increment=>0, end=>$atti_val);
            }else{
            	if(util->isBool(attribute_name=>$atti_name)>0){
            		my @listBoolValues=("True", "False");
					if (grep {$_ eq $atti_val} @listBoolValues) {
						$CurrentAttribute = attribute->new(attribute_name=>$atti_name, start=>$atti_val, increment=>0, end=>$atti_val);
					}else{
						print "Invalid parameter value for $atti_name: $atti_val\n";
						die();
					}
            	}else{
					print "Invalid parameter value for $atti_name: $atti_val\n";
            		die();            		
            	}
            }
            
            push(@attributeList, $CurrentAttribute);
        }
		$count=$count+1;
	}

    for my $attri (@attributeList) {
	    #printf "The Attribute name is %s\n", $attri->getAttributeName();
    }
    
	@{$self->{attributeList}} = @attributeList;
	$self->{attributeList_loaded} = 1;

	#return @attributeList;
	close(TEMPLATE);
}
sub populateConfigFiles {
	my $self = shift;
	die "The template file has not been loaded!!\n" unless $self->{attributeList_loaded}==1;
		
	my @attributeList_obj=@{$self->{attributeList}};
	
    #for my $attri (@attributeList_obj) {
	#    printf "The Attribute name is %s\n", $attri->getAttributeName();
    #}
    
    my $done=0;
    my $test_idx=0;
    
    my $over_flow=0;
    while($over_flow==0){
    	$over_flow=$self->nextAttributeList(test_idx=>$test_idx);
    	print "Overflow is $over_flow\n";
    	$test_idx++;
    }

    #$self->nextAttributeList(test_idx=>$test_idx);
    
    #for my $attri (@attributeList_obj) {
    #    my $atti_name=$attri->getAttributeName();
    #    my $atti_val =$attri->getStartVal();
	#    print "The Attribute name is $atti_name: $atti_val\n";
    #}
    printf "There are %d test cases\n", $test_idx;
}

sub nextAttributeList {
	my $self = shift;
	my %args  = @_;
	my $test_idx=$args{test_idx};
		
	my @attributeList_obj=@{$self->{attributeList}};

	my $name_header="test_";
	my $test_name = $name_header.$test_idx;
	
	my $newTestCase = testCase->new(test_name=>$test_name);
	@{$newTestCase->{attributeList}} = @attributeList_obj;
	$newTestCase->run_test();
	
	
	my $carry=1;
	for my $attri (@attributeList_obj) {
		if($carry==0){
			next;
		}else{
			my $name=$attri->getAttributeName();
			if(util->isBool(attribute_name=>$name)>0){
				if ($attri->getStartVal() eq 'True' && $attri->getEndVal() eq 'False'){
					if($attri->getCurrentVal() eq "True"){
						$attri->currentVal="False";
						$carry=0;
					}elsif($attri->getCurrentVal() eq "False"){
						$attri->currentVal="True";
						$carry=1;
					}
				}else{
					$carry=1;
				}
			}else{
				if ($attri->getStartVal() != $attri->getEndVal()){
					$attri->{currentVal}=$attri->{currentVal}+$attri->getIncrementVal();
					if($attri->{currentVal}>$attri->getEndVal()){
						$attri->{currentVal}=$attri->getStartVal();
						$carry=1;
					}else{
						$carry=0;
					}
				}else{
					$carry=1;
				}

			}
		}
	}
	
	return $carry;
}


1;
