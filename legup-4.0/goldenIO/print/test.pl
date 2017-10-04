#Number of Test Sets
$num_test = $ARGV[1];

$output_file_name = "output.data";

my @array_count;	#Keeps track of which input is array
my @printf;		#used to keep track of input type
open (FILE, "$ARGV[0]");  #function signature

while (<FILE>) {
	chop; # remove newline

	@tokens = split(/--/);
	$num_arg = $#tokens-1;
	$output_type = $tokens[$#tokens];
	$func_name = $tokens[$num_arg];

system ("rm gen_out.c a.out output.data test.c");
system ("cp $func_name.c gen_out.c");
system ("perl genInput.pl $ARGV[0] $num_test >> gen_out.c");
system ("gcc -std=c90 gen_out.c");
system ("./a.out");
system ("cp $func_name.c test.c");
open (TEST, '>>test.c');

	print TEST "#include <stdio.h>\n\n";
	print TEST "int main(void) {\n";
	print TEST "	$output_type ret;\n";
	$count = 0;
	
	#Find input arg types, and declare the variables.
	for ($j = 0; $j < $num_arg; $j++) {

		@const_check = split(/=/, $tokens[$j]);
		@limit_check = split(/</, $const_check[0]);  #used later
		@array_check = split(/:/, $limit_check[0]);
		@arg_tokens = split(/ /, $array_check[0]);


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
        elsif ($arg_tokens[0] eq "float"){
            $float = 1;
            $char = 0;
        }
        elsif ($arg_tokens[0] eq "double"){
            $double = 1;
            $char = 0;
            $float = 0 
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
		if (($char == 1)&&($array_count[$count] == 1)){
			$printf[$count] = "c";
			print TEST "	char arg$count;\n";
		}
		elsif (($char == 1)&&($array_count[$count] != 1)){
			$printf[$count] = "s";
			print TEST "	char* arg$count;\n";
		}
		elsif ($float == 1){
			$printf[$count] = "f";
			if ($array_count[$count] == 1){
				print TEST "	float arg$count;\n";
			}
			else {
				print TEST "	float arg$count\[$array_count[$count]\];\n";
			}
		}
		elsif ($double == 1){
			$printf[$count] = "f";
			if ($array_count[$count] == 1){
				print TEST "	double arg$count;\n";
			}
			else {
				print TEST "	double arg$count\[$array_count[$count]\];\n";
			}
		}
		elsif (($signed == 1)&&($short == 1)){
			$printf[$count] = "hd";
			if ($array_count[$count] == 1){
				print TEST "	short arg$count;\n";
			}
			else {
				print TEST "	short arg$count\[$array_count[$count]\];\n";
			}
		}
		elsif (($signed == 0)&&($short == 1)){
			$printf[$count] = "hu";
			if ($array_count[$count] == 1){
				print TEST "	unsigned short arg$count;\n";
			}
			else {
				print TEST "	unsigned short arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 1)&&($int == 1)){
			$printf[$count] = "d";	
			if ($array_count[$count] == 1){
				print TEST "	int arg$count;\n";
			}
			else {
				print TEST "	int arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 0)&&($int == 1)){
			$printf[$count] = "u";
			if ($array_count[$count] == 1){
				print TEST "	unsigned int arg$count;\n";
			}
			else {
				print TEST "	unsigned int arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 1)&&($long == 1)){
			$printf[$count] = "ld";
			if ($array_count[$count] == 1){
				print TEST "	long arg$count;\n";
			}
			else {
				print TEST "	long arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 0)&&($long == 1)){
			$printf[$count] = "lu";
			if ($array_count[$count] == 1){
				print TEST "	unsigned long arg$count;\n";
			}
			else {
				print TEST "	unsigned long arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 1)&&($longlong == 1)){
			$printf[$count] = "lld";	
			if ($array_count[$count] == 1){
				print TEST "	long long arg$count;\n";
			}
			else {
				print TEST "	long long arg$count\[$array_count[$count]\];\n";
			}
		} 
		elsif (($signed == 0)&&($longlong == 1)){
			$printf[$count] = "llu";
			if ($array_count[$count] == 1){
				print TEST "	unsigned long long arg$count;\n";
			}
			else {
				print TEST "	unsigned long long arg$count\[$array_count[$count]\];\n";
			}
		} 
		$count++;
	}

	print TEST "\n";
}
close (FILE);


open (DATA, 'output.data');
$count = 0;
print TEST "	$output_type output\[$num_test\];\n";
while (<DATA>){
	chomp;
	@tokens = split (/, /);
	$out_index = $#tokens;
	$correct_output[$count] = $tokens[$out_index];
	
	$arg_index = 0;
	$data_index = 0;
	for ($i = 0; $i <= $#array_count; $i++){
		if (($array_count[$i] == 1)||($printf[$i] eq "s")) {
			if ($printf[$i] eq "c"){
				print TEST "	arg$arg_index = '$tokens[$data_index]';\n";
				$data_index++;
			}
			elsif ($printf[$i] eq "s"){
				print TEST "	arg$arg_index = \"$tokens[$data_index]\";\n";
				$data_index++;
			}
			else {
				print TEST "	arg$arg_index = $tokens[$data_index];\n";
				$data_index++;
			}
		}
		else {
			for ($p = 0; $p < $array_count[$i]; $p++){
				print TEST "	arg$arg_index\[$p\] = $tokens[$data_index];\n";
				$data_index++;
			}
		}
		$arg_index++;

	}
	print TEST "	output[$count] = $func_name(";
	for ($i = 0; $i <= $#array_count; $i++){
		print TEST "arg$i";
		if ($i == $#array_count){
			print TEST ");\n\n";
		}
		else {
			print TEST ", ";
		}
	}
	$count++;
}
close (DATA);

print TEST "\n	$output_type correct_output[$num_test] = {";
for ($j = 0; $j <= $#correct_output; $j++){
	print TEST ("$correct_output[$j]");
	if ($j < $#correct_output){
		print TEST (", ");
	}
}
print TEST ("};\n\n");

print TEST ("	int i;\n");
print TEST ("	int count = 0;\n");
print TEST ("	for (i=0; i < $num_test; i++){\n");
print TEST ("		if (output[i] == correct_output[i]){\n");
print TEST ("			count++;\n");
print TEST ("		}\n");
print TEST ("	}\n");
print TEST ("	printf(\"PASSED:\%d out of $num_test\\n\", count);\n");
print TEST ("	return count;\n");
print TEST ("}\n");

close (TEST);

system ("gcc -std=c90 test.c");
system ("./a.out");
