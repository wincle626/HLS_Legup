#include <iostream>
#include <fstream>
#include <string>
#include "utils.h"
#include <stdlib.h>
#include <limits.h>
#include <glib.h>
#include <string.h>

using namespace std;

GHashTable* addrsHashTable = g_hash_table_new(g_str_hash, g_str_equal);

// argv[1] is the instruction trace file
// argv[2] is the number of accesses above which the address will be printed out.
//         ex if you choose 10, then all the addresses where more than 10 different functions access
//         it will be printed out
// argv[3] is the name of the function you want to analyze
int main(int argc,char* argv[]){

    cout<<" - - - - Memory Access Profiler - - - -"<<endl;
	cout<<" Usage: ./function_profiler <instruction trace file name> <number> <function name>"<<endl;
	cout<<" When the # of accesses to an address is greater than the <number> given above, the accesses will be saved to the file."<<endl;
	cout<<"    Function Name: "<<argv[3]<<endl;
	cout<<"    Building the lists of accesses..."<<endl;
    
    ifstream inputFile;
    inputFile.open(argv[1]);

    char lineFromFile_cstr[1024];
    string lineFromFile; // Keep one line from the file
    string currentCallPath; // Keep the function call path. ex main>myfunction>printf
    string address;
    int begin,end, line_number = 0;
    char access_type; //'w' for write, 'r' for read
    address_node* curr_struct;
    int function_id = 1;
	int function_invocation_count = 0;
    string currentInvocation;
    unsigned int stack_ptr = 0xa0007f00; //sp starts at 0xa0007f00
    //int stack_range = 0;
	int node_count = 0;
	address_node* list_of_addresses = NULL;
    
	// Keep track of the a0,a1,a2,a3 registers. These registers are used to pass in values to functions.
	register_node arg_registers[4];
	
	//register_node return_registers[2];
	
	
/*
 /// This section was going to be for finding the lowest sp value to find the DEFINITE sp and heap address ranges
 /// However, just looking at 800 vs a00 seems to be safe enough for most programs...
    unsigned int sp_min = UINT_MAX;
    /// First find the lowest sp value. This will let us determine
    /// whether a given address is in the stack or in the heap
    while (!inputFile.eof()){
        inputFile.getline(lineFromFile_cstr,1024);
        lineFromFile = lineFromFile_cstr;
        int index = lineFromFile.find("sp = ");
        if (index != -1){
            cout<<lineFromFile<<endl;
            unsigned int temp = hexStrToInt (lineFromFile.substr(index+15,8).c_str());
            if (temp < sp_min) sp_min = temp;
        }
    }
    cout<<sp_min<<endl;

    inputFile.close();
    inputFile.open(argv[1]);
*/

	// Go through all the instructions, build up a hash table of all the accessed memory addresses.
	// For each memory address, build a list of all the functions that access it
    while (!inputFile.eof()){
        inputFile.getline(lineFromFile_cstr,1024);
        lineFromFile = lineFromFile_cstr;
        line_number++;

        /// Check if under a new function
        if (lineFromFile.find(")>") != string::npos && lineFromFile.find("<") != string::npos){ // if under a new function

            // Extract the function's name.
            begin = lineFromFile.find_first_of("<");
            end = lineFromFile.find_first_of("(");

            string functionName = lineFromFile.substr(begin+1,end-begin-1);
            string prevCallPath = currentCallPath;
            
            
            // Determine the depth of this function call (in gxemul output, every 2 spaces = 1 deep)
            if (begin == 0){ //this is the top level function (probably the main)
                currentCallPath = functionName;
                currentCallPath.push_back('>');
            } else {
                // If not the top level function, need to modify the function call path accordingly.
                // find the position of the '>' to replace. ex "main>myfunction>anotherfunction"
                // if current depth is 1, need to replace from main> onwards.
                int pos = 0;
                int count = 0;
                for (pos = 0; currentCallPath[pos] != '\0'; pos++){
                    if (currentCallPath[pos] == '>') count++;
                    if (count == begin/2) break;
                }
                               
                //cout<<"pos: "<<pos<<" begin/2: "<<begin/2<<endl;
                // if gxemul screwed up the output (want to go 2 function call levels down) just append it (treat it as 1 level down).
                if (pos == -1){
                    cout<<"WARNING: possible call path issue at line: "<<line_number<<endl;
                    currentCallPath.append(functionName);
                    currentCallPath.push_back('>');
                // otherwise do the proper
                } else {
                    // If gxemul wants to go down 2 or more levels (error in gxemul) treat it as 1 level down, but give warning.
                    if (count < begin/2) cout<<"WARNING: possible call path issue at line: "<<line_number<<endl;
                    currentCallPath.erase(pos,string::npos);
                    currentCallPath.push_back('>');
                    currentCallPath.append(functionName);
                    currentCallPath.push_back('>');
                }
            }
        
            // Figure out whether to update function_id.
            // (basically need to figure out if we are still within a single invocation of the function we are interested in)
            // ex: if we are interested in function "float_add" then going from 
            // main>float_add>   to     main>float_add>addfunction>
            // shouldn't cause an update, because they are still under the same invocation of float_add
            // main>float_add>   to     main>float_sub>   
            // SHOULD cause an update because it is a totally different invocation
            
            // If come across a call path containing the function in question
            if (currentCallPath.find(argv[3]) != string::npos && currentInvocation.empty()){
                currentInvocation = currentCallPath;
				function_invocation_count++;
                function_id++;
	    // If new path is a decendant, don't update
            } else if (currentCallPath.find(currentInvocation) != string::npos && currentCallPath != currentInvocation ){ 
                function_id = function_id;
            // If the same function call has been invoked again
            } else if (currentCallPath == currentInvocation){
				function_invocation_count++;
                function_id++;
            // If the function in question has been invoked in a totally different call path, update function id
	    } else if (currentCallPath.find(currentInvocation) == string::npos && currentCallPath.find(argv[3]) != string::npos){
                function_invocation_count++;
				function_id++;
            // If none of the above is true, then this invocation does not involve our function
            } else {
                currentInvocation.clear();
                //if (currentCallPath.find(argv[3]) != string::npos) currentInvocation = currentCallPath;
                
                function_id++;
            }
            //cout<<currentCallPath<<" "<<function_id<<endl;
            // Done with this line, continue to next line.
            continue;
        }
        
        /// Check if a function returns
        if (lineFromFile.find("\tjr\tra") != string::npos){
            int index = currentCallPath.find_last_of('>',currentCallPath.length()-2);
            // If index < 0 that means we are at the end of the instruction trace (tries to go from "main>" to "")
            if (index < 0) break;
            currentCallPath.erase(index+1);
            //cout<<currentCallPath<<" "<<function_id<<endl;
            
        }
        
      
        
        /// Check if this line contains a local variable declaration (pushes into the stack)
        // if line contains     addiu   sp,sp,<negative number>   it is increasing the stack size
        if (lineFromFile.find("\taddiu\tsp,sp,-") != string::npos){
            int index = lineFromFile.find("\taddiu\tsp,sp,-");
            // get the decrement value
            int decrement_value = atoi(lineFromFile.substr(index+14).c_str());
            
            // add the addresses to the list (each byte address)
            // add a "dummy" access to each of these addresses. a dummy
            // address is simply to note that this locality has touched the address,
            // but set the # of accesses to 0 and # of invocations accessed to 0.
            // This is important because if a variable is declared but not initialized,
            // then in the instruction trace there will be no sw/lw to that address.
            // The only way to detect such variable declarations is the look at sp
            for (int i = 0; i < decrement_value; i++){
                stack_ptr--;
				//cout<<stack_ptr<<endl;
                char address_str[16];
                sprintf (address_str, "%x",stack_ptr);
				
                //curr_struct = find_address(list_of_addresses,address_str);
                // Look for the address in the hash table
				address_node* temp = (address_node*) g_hash_table_lookup(addrsHashTable,address_str);
				
                // If this is the first time this address has been accessed
                if (temp == NULL){
						
					// Add the address to the HASH TABLE and add the access
					g_hash_table_insert(addrsHashTable, g_strdup(address_str),new address_node);
					node_count++;
					//cout<<node_count<<endl;
					//cout<<address_str<<endl;
					address_node* temp = (address_node*) g_hash_table_lookup(addrsHashTable,address_str);
					temp ->address = address_str;
					temp->add_access(currentCallPath,STACKWRITE,function_id,false);

                // If address already exists
                } else {
					// Add the access
                    temp->add_access(currentCallPath,STACKWRITE,function_id,false);
                }
            }
            continue;
        }
        /// Chcek if this line contains a stack pop
        if (lineFromFile.find("\taddiu\tsp,sp,") != string::npos){
            int index = lineFromFile.find("\taddiu\tsp,sp,");
            // get the increment value
            int increment_value = atoi(lineFromFile.substr(index+13).c_str());
            stack_ptr = stack_ptr + increment_value;
            continue;
        }

        access_type = 0;
        /// Check if this line contains a memory access
        // if line contains "sw" or "sb" it is a write
        if (lineFromFile.find("\tsw\t") != string::npos || 
			lineFromFile.find("\tsb\t") != string::npos ||
			lineFromFile.find("\tsh\t") != string::npos
		){
            // Check if occurs in stack. NOTE THESE IS A FIRST-CUT SOLUTION. NEED TO FIND LOWEST SP VALUE.
            if(lineFromFile.find("[0xffffffffa0") != string::npos) access_type = STACKWRITE;
            // Check if occurs in heap
            else if (lineFromFile.find("[0xffffffff80") != string::npos) access_type = HEAPWRITE;
            else {
                cout<<"Warning: line with improper address found. Ignoring this access. Line: "<<line_number<<endl;
                continue;
            }
        // if line contains "lw" or "lb" or "lbu" it is a read
        }else if (lineFromFile.find("\tlw\t") != string::npos || 
                  lineFromFile.find("\tlb\t") != string::npos ||
                  lineFromFile.find("\tlbu\t") != string::npos ||
                  lineFromFile.find("\tlh\t") != string::npos ||
                  lineFromFile.find("\tlhu\t") != string::npos   
        ){
            // Check if occurs in stack. NOTE THESE IS A FIRST-CUT SOLUTION. NEED TO FIND LOWEST SP VALUE.
            if(lineFromFile.find("[0xffffffffa0") != string::npos) access_type = STACKREAD;
            // Check if occurs in heap.
            else if (lineFromFile.find("[0xffffffff80") != string::npos) access_type = HEAPREAD;
            else {
                cout<<"Warning: line with improper address found. Ignoring this access. Line: "<<line_number<<endl;
                continue;
            }
        // Not read nor write
        } else
            continue; // no read or write. continue to next line.

        /// If this memory access was dealing with saving/getting the return address, ignore this access.
        if (lineFromFile.find("\tra,") != string::npos) continue;

        /// By this point, a read or write has occured. Get the address accessed.
        begin = lineFromFile.find("[0x");
        end = lineFromFile.find("]");
        // Sometimes gxemul's output screws up. Give a warning
        if (begin == -1 || end == -1 || begin+18 > end ){
            cout<<"Warning: line with improper address found. Ignoring this access. Line: "<<line_number<<endl;
            continue;
        }
        address = lineFromFile.substr(begin+11,8);

		/// Check if this is a load to a0 or a1 or a2 or a3.
        if (lineFromFile.find("\tlw\ta0") != string::npos || 
			lineFromFile.find("\tlb\ta0") != string::npos ||
			lineFromFile.find("\tlb\ta0") != string::npos || 
			lineFromFile.find("\tlh\ta0") != string::npos ||
			lineFromFile.find("\tlhu\ta0") != string::npos )
		{
			arg_registers[0].written_by = currentCallPath;
			arg_registers[0].associated_address = address;
		}
		if (lineFromFile.find("\tlw\ta1") != string::npos || 
			lineFromFile.find("\tlb\ta1") != string::npos ||
			lineFromFile.find("\tlbu\ta1") != string::npos|| 
			lineFromFile.find("\tlh\ta1") != string::npos ||
			lineFromFile.find("\tlhu\ta1") != string::npos )
		{
			arg_registers[1].written_by = currentCallPath;
			arg_registers[1].associated_address = address;
		}
		if (lineFromFile.find("\tlw\ta2") != string::npos || 
			lineFromFile.find("\tlb\ta2") != string::npos ||
			lineFromFile.find("\tlbu\ta2") != string::npos|| 
			lineFromFile.find("\tlh\ta2") != string::npos ||
			lineFromFile.find("\tlhu\ta2") != string::npos  )
		{
			arg_registers[2].written_by = currentCallPath;
			arg_registers[2].associated_address = address;
		}
		if (lineFromFile.find("\tlw\ta3") != string::npos || 
			lineFromFile.find("\tlb\ta3") != string::npos ||
			lineFromFile.find("\tlbu\ta3") != string::npos|| 
			lineFromFile.find("\tlh\ta3") != string::npos ||
			lineFromFile.find("\tlhu\ta3") != string::npos )
		{
			arg_registers[3].written_by = currentCallPath;
			arg_registers[3].associated_address = address;
		}
		
		/// Check if this is a read from a0 or a1 or a2 or a3.
		// In MIPS, when variables are passed in with a0-a3, the called function first
		// saves them onto the stack with sw. This means that we can simply looks for
		// sw a0 or sw a1, etc. to find reads from the registers (seems awkward but this works).
		// When this happens, treat it like it's a read from the address associated with that register
        if ((lineFromFile.find("\tsw\ta0") != string::npos || 
			lineFromFile.find("\tsb\ta0") != string::npos  || 
			lineFromFile.find("\tsh\ta0") != string::npos) &&
			arg_registers[0].associated_address.empty() == false)
		{
			curr_struct = (address_node*) g_hash_table_lookup(addrsHashTable,arg_registers[0].associated_address.c_str());
			if (curr_struct != NULL) curr_struct->add_access(currentCallPath,STACKREAD,function_id,true);
		}
		if ((lineFromFile.find("\tsw\ta1") != string::npos || 
			lineFromFile.find("\tsb\ta1") != string::npos  || 
			lineFromFile.find("\tsh\ta1") != string::npos) &&
			arg_registers[0].associated_address.empty() == false)
		{
			curr_struct = (address_node*) g_hash_table_lookup(addrsHashTable,arg_registers[1].associated_address.c_str());
			if (curr_struct != NULL) curr_struct->add_access(currentCallPath,STACKREAD,function_id,true);
		}
		if ((lineFromFile.find("\tsw\ta2") != string::npos || 
			lineFromFile.find("\tsb\ta2") != string::npos  || 
			lineFromFile.find("\tsh\ta2") != string::npos) &&
			arg_registers[0].associated_address.empty() == false)
		{
			curr_struct = (address_node*) g_hash_table_lookup(addrsHashTable,arg_registers[2].associated_address.c_str());
			if (curr_struct != NULL) curr_struct->add_access(currentCallPath,STACKREAD,function_id,true);
		}
		if ((lineFromFile.find("\tsw\ta3") != string::npos || 
			lineFromFile.find("\tsb\ta3") != string::npos  || 
			lineFromFile.find("\tsh\ta3") != string::npos) &&
			arg_registers[0].associated_address.empty() == false)
		{
			curr_struct = (address_node*) g_hash_table_lookup(addrsHashTable,arg_registers[3].associated_address.c_str());
			if (curr_struct != NULL) curr_struct->add_access(currentCallPath,STACKREAD,function_id,true);
		}
		/* THIS CHECK HAS BEEN DISABLED TO FIND MORE PARALELLIZABLE FUNCTIONS
		 * In case there functions whose return values are written to the same variable
		 * where the variable is just being reused (like int temp). In those cases
		 * we can just modify the code so that they write to different variables
		 * This check would have said NON-parallelizable because the return values are written to the same var. */
		/*
		/// If this is a return value, check where it gets written to! if 2 functions return values
		/// and the values get written to the same variable, they are not parallelizable.
		/// This doesn't get picked up by existing checks because the return value is written AFTER
		/// returning to the parent function.
		// Return value is loaded to the register
		if (lineFromFile.find("\tlw\tv0") != string::npos || 
			lineFromFile.find("\tlb\tv0") != string::npos || 
			lineFromFile.find("\tlbu\tv0") != string::npos|| 
			lineFromFile.find("\tlh\tv0") != string::npos || 
			lineFromFile.find("\tlhu\tv0") != string::npos)
		{
			return_registers[0].written_by = currentCallPath;
			return_registers[0].associated_address = address;
			return_registers[0].func_id = function_id;
		}
		if (lineFromFile.find("\tlw\tv1") != string::npos || 
			lineFromFile.find("\tlb\tv1") != string::npos || 
			lineFromFile.find("\tlbu\tv1") != string::npos|| 
			lineFromFile.find("\tlh\tv1") != string::npos || 
			lineFromFile.find("\tlhu\tv1") != string::npos)
		{
			return_registers[1].written_by = currentCallPath;
			return_registers[1].associated_address = address;
			return_registers[1].func_id = function_id;
		}
		// Return value is read from the return register and stored somewhere
		if ((lineFromFile.find("\tsw\tv0") != string::npos || 
			lineFromFile.find("\tsb\tv0") != string::npos  || 
			lineFromFile.find("\tsh\tv0") != string::npos) &&
			return_registers[0].written_by.empty() == false)
		{
			curr_struct = (address_node*) g_hash_table_lookup(addrsHashTable,address.c_str());
			if (curr_struct != NULL) curr_struct->add_access(return_registers[0].written_by,access_type,return_registers[0].func_id,true);
		
			continue;
		}
		if ((lineFromFile.find("\tsw\tv1") != string::npos || 
			lineFromFile.find("\tsb\tv1") != string::npos  || 
			lineFromFile.find("\tsh\tv1") != string::npos) &&
			return_registers[1].written_by.empty() == false)
		{
			curr_struct = (address_node*) g_hash_table_lookup(addrsHashTable,address.c_str());
			if (curr_struct != NULL) curr_struct->add_access(return_registers[1].written_by,access_type,return_registers[1].func_id,true);
		
			continue;
		}
		*/
		
        /// By this point, sb/sw/lb/lw has occured AND address has been found. See if this address already exists in our list
		curr_struct = (address_node*) g_hash_table_lookup(addrsHashTable,address.c_str());
		//cout<<"currstruct: "<<curr_struct<<endl;
        // If this is the first time this address has been accessed, add this address node
        if (curr_struct == NULL){
			g_hash_table_insert(addrsHashTable,g_strdup(address.c_str()),new address_node);
			node_count++;
			address_node* temp = (address_node*) g_hash_table_lookup(addrsHashTable,address.c_str());
			temp ->address = address.c_str();
			temp->add_access(currentCallPath,access_type,function_id,true);
        } else {
			curr_struct->add_access(currentCallPath,access_type,function_id,true);
		}
    }
    
    // By this point, we have compiled the table of all the addresses and the functions that access them
    cout<<"    List of accesses complete."<<endl;
    
    GList* temp =  g_hash_table_get_values(addrsHashTable);
    string print_options = "  "; // no options.
	list_of_addresses = (address_node*)temp->data;
    while (temp != NULL){
		// construct the linked list of address nodes to pass to find_conflicts()
		if (temp->next != NULL){
			((address_node*)temp->data)->next = (address_node*)(temp->next->data);
		} else {
			((address_node*)temp->data)->next = NULL;
		}
		
		// print out the information at the  requested addresses
        if (((address_node*)temp->data)->different_functions_accessed > atoi(argv[2]))
            ((address_node*)temp->data)->print_accessing_functions(print_options);
        temp = temp->next;
    }

	cout<<"    Running parallelizeable checker."<<endl;
	
    //cout<<"Parallelizable: "<<parallelizable (list_of_addresses, argv[3])<<endl;
    parallelizable (list_of_addresses, argv[3], argv[1]);
    //string conflict_options = "nostackread nostackwrite nostackread";
    //cout<<"#of conflicts: "<<find_conflicts(list_of_addresses,"subFloat64Sigs","subFloat64Sigs",2,conflict_options)<<endl;

	ofstream outputFile;
	string file_name = argv[1];
	file_name.append("_");
	file_name.append(argv[3]);
	file_name.append("_parallelize_analysis.txt");
    outputFile.open(file_name.c_str(),ios_base::app);
	outputFile<<endl;
	outputFile<<"Invocation count of this function: "<<function_invocation_count<<endl;
	
	
	outputFile.close();
	
	cout<<"    Saved results to: "<<argv[1]<<"_"<<argv[3]<<"_parallelizability_analysis.txt"<<endl;
	
/*
	address_node* temp = (address_node*) g_hash_table_lookup(addrsHashTable,"a0007efc");
	if (temp != NULL) cout<<temp->address<<endl;
*/
    inputFile.close();
    return 0;
}


