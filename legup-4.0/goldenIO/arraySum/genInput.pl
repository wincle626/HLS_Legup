#If user does not specify limit or constant value for variables, these limit will be used according to the data type.  Comment on the right is the lower and upper limit for the data type. 
#Signed limit will apply both positive and negative.  Ex. if signed limit is 10000, the limit will be -10000 to 10000

#short-16bit
$short_signed_limit = 30000; # -32,768 to 32,767
$short_unsigned_limit = 65000; # 0 to 65,535
#int-32bit
$int_signed_limit = 2000000000; # -2,147,483,648 to 2,147,483,647
$int_unsigned_limit = 4000000000; # 0 to 4,000,000,000
#long-32bit
$long_signed_limit = 2000000000; # -2,147,483,648 to 2,147,483,647
$long_unsigned_limit = 4000000000; # 0 to 4,000,000,000
#long-64bit
$longlong_signed_limit =1000000000000; # -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
$longlong_unsigned_limit = 2000000000000; # 0 to 18,446,744,073,709,551,615

#Golden input/output output file name
$output_file_name = "output.data";

#Number of Test Sets
$num_test = $ARGV[1];

my @array_count;		#Keeps track of which input is array
my @limit;				#Limit used for each input
my @printf;				#Flag used used for printf for convenience
my @const;				#Keeps track of which input is a constanct (0-use limit, 1-use const_value)
my @const_value;		#The actual constant value


#Subfunction used to generate random character/string.
#Use: $string = &generate_random_string(<# of characters>);
sub generate_random_string
{
	my $length_of_randomstring=shift;# the length of the random string to generate

	my @chars=('a'..'z','A'..'Z','0'..'9','_');
	my $random_string;
	foreach (1..$length_of_randomstring) 
	{
		# rand @chars will generate a random 
		# number between 0 and scalar @chars
		$random_string.=$chars[rand @chars];
	}
	return $random_string;
}

open (FILE, "$ARGV[0]");  #function signature

while (<FILE>) {
	chop; # remove newline

	@tokens = split(/--/);
	$num_arg = $#tokens-1;
	$output_type = $tokens[$#tokens];
	$func_name = $tokens[$num_arg];

	print "#include <stdio.h>\n\n";
	print "int main(void) {\n";
	print "	$output_type ret;\n";
	$count = 0;
	print "	FILE *fp;\n";
	print "	if \(fp = fopen\(\"$output_file_name\", \"w\"\)\){\n\n";
	
	#Find input arg types, and declare the variables.
	for ($j = 0; $j < $num_arg; $j++) {

		@const_check = split(/=/, $tokens[$j]);
		@limit_check = split(/</, $const_check[0]);  #used later
		@array_check = split(/:/, $limit_check[0]);
		@arg_tokens = split(/ /, $array_check[0]);

	#If split by = produced 2 tokens for const_check, the user has specified a constant for the variable.
		if ($#const_check == 1){
			$const[$count] = 1;
			$const_value[$count] = $const_check[1];
		}
		else{
			$const[$count] = 0;
		}

	#If split by : produced 2 tokens for array_check, the user has specified a constant for the variable.
		if ($#array_check == 0){
			$array_count[$count] = 1;
		}
		else{
			$array_count[$count] = $array_check[1];
		}

	#check if each variable is char, unsigned, short, int, long or long long
		if ($arg_tokens[0] eq "char"){
			$char = 1;
		}
		else{
			$char = 0;

		#The only time the variable will be unsigned is if the first word is "unsigned"
			if ($arg_tokens[0] eq "unsigned"){
				$signed = 0;
			}
			else{
				$signed = 1;
			}

		#when the variable is short, it must be "short (int)", "signed short (int)", "unsigned short (int)".  If the first or second word is "short", the variable must be short type
			if (($arg_tokens[0] eq "short")||($arg_tokens[1] eq "short")){
				$short = 1;
			}
			else{
				$short = 0;
			}
		
		#count the number of time "long" appears in variable name.
		#if 2, must be long long
		#if 1, must be long
		#if 0 and not short or char, must be int
			$long_count = 0;
			for ($k = 0; $k <=$#arg_tokens; $k++){
				if ($arg_tokens[$k] eq "long"){
					$long_count++;
				}
			}

			if ($long_count == 2){
				$longlong = 1;
				$long = 0;
				$int = 0;
			}
			elsif ($long_count == 1){
				$longlong = 0;
				$long = 1;
				$int = 0;
			}
			elsif (($short == 0)&&($char == 0)){
				$longlong = 0;
				$long = 0;
				$int = 1;
			}
		}

	#using the data type and array info, declare the variable.
	#printf and limit holds the flag used in printf and limit used to generate random input respectively.  
		if (($char == 1)&&($array_count[$count] == 1)){
			$printf[$count] = "c";
			$limit[$count] = 1;
			print "	char arg$count;\n";
		}
		elsif (($char == 1)&&($array_count[$count] != 1)){
			$printf[$count] = "s";
			$limit[$count] = 1;
			print "	char* arg$count;\n";
		}
		elsif (($signed == 1)&&($short == 1)){
			$printf[$count] = "hd";
			$limit[$count] = $short_signed_limit;	
			if ($array_count[$count] == 1){
				print "	short arg$count;\n";
			}
			else {
				print "	short arg$count\[$array_count[$count]\];\n";
			}
		}
		elsif (($signed == 0)&&($short == 1)){
			$printf[$count] = "hu";
			$limit[$count] = $short_unsigned_limit;	
			if ($array_count[$count] == 1){
				print "	unsigned short arg$count;\n";
			}
			else {
				print "	unsigned short arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 1)&&($int == 1)){
			$printf[$count] = "d";
			$limit[$count] = $int_signed_limit;	
			if ($array_count[$count] == 1){
				print "	int arg$count;\n";
			}
			else {
				print "	int arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 0)&&($int == 1)){
			$printf[$count] = "u";
			$limit[$count] = $int_unsigned_limit;	
			if ($array_count[$count] == 1){
				print "	unsigned int arg$count;\n";
			}
			else {
				print "	unsigned int arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 1)&&($long == 1)){
			$printf[$count] = "ld";
			$limit[$count] = $long_signed_limit;	
			if ($array_count[$count] == 1){
				print "	long arg$count;\n";
			}
			else {
				print "	long arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 0)&&($long == 1)){
			$printf[$count] = "lu";
			$limit[$count] = $long_unsigned_limit;	
			if ($array_count[$count] == 1){
				print "	unsigned long arg$count;\n";
			}
			else {
				print "	unsigned long arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 1)&&($longlong == 1)){
			$printf[$count] = "lld";
			$limit[$count] = $longlong_signed_limit;	
			if ($array_count[$count] == 1){
				print "	long long arg$count;\n";
			}
			else {
				print "	long long arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 0)&&($longlong == 1)){
			$printf[$count] = "llu";
			$limit[$count] = $longlong_unsigned_limit;
			if ($array_count[$count] == 1){
				print "	unsigned long long arg$count;\n";
			}
			else {
				print "	unsigned long long arg$count\[$array_count[$count]\];\n";
			}
		} 
	#if split by < produced 2 tokens, use have specified a custom limit for the variable-overwrite the limit that will be used.
		if ($#limit_check == 1){
			$limit[$count] = $limit_check[1];
		}
		$count++;
	}

	print "\n";

#check output type.  Same logic as input.
		$char = 0;
		$signed = 0;
		$short = 0;
		$int = 0;
		$long = 0;
		$longlong = 0;
		@output = split(/ /, $output_type);
		if ($output[0] eq "char"){
			$out_printf = "c";
		}
		else{
			if ($output[0] eq "unsigned"){
				$signed = 0;
				#print "unsigned";
			}
			else{
				$signed = 1;
				#print "unsigned";
			}

			if (($output[0] eq "short")||($output[1] eq "short")){
				$short = 1;
				#print "short";
			}
			else{
				$short = 0;
				#print "not short";
			}
			
			$long_count = 0;
			for ($k = 0; $k <=$#output; $k++){
				if ($output[$k] eq "long"){
					$long_count++;
				}
			}

			if ($long_count == 2){
				$longlong = 1;
				$long = 0;
				$int = 0;
				#print "long long";
			}
			elsif ($long_count == 1){
				$longlong = 0;
				$long = 1;
				$int = 0;
				#print "long";
			}
			elsif ($short == 0){
				$longlong = 0;
				$long = 0;
				$int = 1;
				#print "long";
			}
		}

		if ($signed == 1){
			if ($short == 1){
				$out_printf = "hd";
			}
			elsif ($int == 1){
				$out_printf = "d";
			}
			elsif ($long == 1){
				$out_printf = "ld";
			}
			else{
				$out_printf = "lld";
			}
		}
		else{
			if ($short == 1){
				$out_printf = "hu";
			}
			elsif ($int == 1){
				$out_printf = "u";
			}
			elsif ($long == 1){
				$out_printf = "lu";
			}
			else{
				$out_printf = "llu";
			}
		}


#Generate random number/character/string
	for ($i = 0; $i < $num_test; $i++) {
		for ($a = 0; $a < $num_arg; $a++){
		#number
			if (($array_count[$a] == 1)&&($printf[$a] ne "c")&&($printf[$a] ne "s")){
				$randInt = int(rand($limit[$a]));
			#If the number is signed, generate 1 or 2 randomly.  If 1, turn the randomly generate nuber into negative.
				if (($printf[$a] eq "hd")||($printf[$a] eq "d")||($printf[$a] eq "ld")||($printf[$a] eq "lld")){
					$neg = int(rand(2));
					if ($neg == 1){
						$randInt = 0 - $randInt;
					}
				}
			#check if the input is specified to be a constant.  If so, overwrite the random number
				if ($const[$a] == 1){
					$randInt = $const_value[$a];
				}
				print "	arg$a = $randInt;\n";
			}
			#number array
			elsif (($array_count[$a] != 1)&&($printf[$a] ne "c")&&($printf[$a] ne "s")){
				for ($c = 0; $c < $array_count[$a]; $c++){
					print "	arg$a\[$c] = ";
					$randInt = int(rand($limit[$a]));
					if (($printf[$a] eq "hd")||($printf[$a] eq "d")||($printf[$a] eq "ld")||($printf[$a] eq "lld")){
						$neg = int(rand(2));
						if ($neg == 1){
							$randInt = 0 - $randInt;
						}
					}
					if ($const[$a] == 1){
						$randInt = $const_value[$a];
					}
					print "$randInt;\n";
				}
			}
			#character or string
			elsif (($printf[$a] eq "c")||($printf[$a] eq "s")){
				$string = &generate_random_string($array_count[$a]);
				if ($const[$a] == 1){
					$string = $const_value[$a];
				}
				if ($array_count[$a] == 1){
					print "	arg$a = \'$string\';\n";  #single quote for character
				}
				else{
					print "	arg$a = \"$string\";\n";  #double quote for string
				}
			}
		}
	#Call the function with all arguments
		print "	ret = $func_name(";
		for ($b = 0; $b < $num_arg; $b++){
			print ("arg$b");
			if ($b != ($num_arg-1)){
				print (", ");
			}
			else{
				print (");\n\n");
			}
		}

#Produce output.data
		print "	fprintf(fp, \"";
		for ($d = 0; $d < $num_arg; $d++){
			if (($printf[$d] eq "c")||($printf[$d] eq "s")||($array_count[$d] == 1)){
				print "%$printf[$d]"
			}
			else{
				for ($e = 0; $e < $array_count[$d]; $e++){
					print "%$printf[$d]";
					if ($e != ($array_count[$d]-1)){
						print ", ";
					}
				}
			}
			if ($d != ($num_arg-1)){
				print ", ";
			}
			else {
				print ", %$out_printf\\n\", ";
			}
		}
		for ($f = 0; $f < $num_arg; $f++){
			if (($printf[$f] eq "c")||($printf[$f] eq "s")||($array_count[$f] == 1)){
				print "arg$f"
			}
			else{
				for ($g = 0; $g < $array_count[$f]; $g++){
					print "arg$f\[$g\]";
					if ($g != ($array_count[$f]-1)){
						print ", ";
					}
				}
			}
			if ($f != ($num_arg-1)){
				print ", ";
			}
			else {
				print ", ret);\n\n";
			}
		}
	}
	print "	}\n";
	print "	else\n";
	print "	printf(\"Error opening output.data\\n\");\n\n";
}
close (FILE);
print "	return 0;\n}\n";
