#include "utils.h"
#include <string.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <fstream>

unsigned int hexStrToInt (const char* str){
    size_t strLength = strlen(str);
    unsigned int result = 0;

    int i;
    int curr;
    for (i=strLength-1; i>=0; i--){
        if (str[i] == '0') curr = 0;
        else if (str[i] == '1') curr = 1;
        else if (str[i] == '2') curr = 2;
        else if (str[i] == '3') curr = 3;
        else if (str[i] == '4') curr = 4;
        else if (str[i] == '5') curr = 5;
        else if (str[i] == '6') curr = 6;
        else if (str[i] == '7') curr = 7;
        else if (str[i] == '8') curr = 8;
        else if (str[i] == '9') curr = 9;
        else if (str[i] == 'a' || str[i] == 'A') curr = 10;
        else if (str[i] == 'b' || str[i] == 'B') curr = 11;
        else if (str[i] == 'c' || str[i] == 'C') curr = 12;
        else if (str[i] == 'd' || str[i] == 'D') curr = 13;
        else if (str[i] == 'e' || str[i] == 'E') curr = 14;
        else if (str[i] == 'f' || str[i] == 'F') curr = 15;
        else return 0;

        result+= curr*pow(16,strLength-i-1);
    }

    return result;

}

 address_node* find_address( address_node* list, const char* address){
    // empty list
    if (list == NULL){
        return NULL;
    }

     address_node* curr = list;
    while (1){
        if (curr->address == address){
            return curr;
        }

        if (curr->next == NULL) break;
        else curr = curr->next;
    }
    // the address has not been found.
    return NULL;
}

/// Return the index of the array that contains the previous access of the address by this function.
/// Return -1 if never occured.
int address_node::find_function (string function_name, char access_type){
    for (int i = 0; i < MAX_FUNCTIONS && this->access_count[i] != -1; i++){
        if (this->accessing_functions[i] == function_name && this->access_type[i] == access_type) return i;
    }


    return -1;
}

bool address_node::previously_accessed (string function_name){
    for (int i = 0; i < MAX_FUNCTIONS; i++){
        if (accessing_functions[i] == function_name) return true;
    }

    return false;
}

address_node::address_node(){
    for (int i = 0; i < MAX_FUNCTIONS; i++){
        this->access_type[i] = 'n';
        this->access_count[i] = -1;
        this->function_invocations[i] = 0;
        //this->newinvocation[i] = true;
    }
    this->empty_index = 0;
    this->next = NULL;
    this->different_functions_accessed = 0;
}

// If increment = 1, then increment. if 0, then add a dummy access (with access count = 0)
void address_node::add_access(string function_name, char access_type, int function_id, bool increment){

    int index = find_function(function_name, access_type);
    // This function has accessed this address before.
    // If function has accessed before, then no need to add a dummy access... It's already there!
    if (index >= 0){
		if (increment){
			access_count[index]++;
			/*
			if (newinvocation[index]){
				accessor_id[index] = function_id;
				newinvocation[index] = false;
				function_invocations[index]++;
			}*/
			// If a new invocation is accessing it
			if (accessor_id[index] != function_id){
				accessor_id[index] = function_id;
				function_invocations[index] ++;
			}
		}
    // This function's first time accessing this address with the specific access type
    } else {
        if (increment){
            // Check if this is the first read/write. if yes, increment
            // We do this because read and write from the same function should only contribute
            // +1 to different_functions_accessed, not +2.
            if (previously_accessed(function_name) == false)
                different_functions_accessed++;

            //index = get_first_empty_index();
            index = empty_index;
            empty_index++;
            accessing_functions[index] = function_name;
            this->access_type[index] = access_type;
            access_count[index] = 1;
            function_invocations[index] = 1;
            accessor_id[index] = function_id;
        // Add the dummy access
        } else {
            //index = get_first_empty_index();
            index = empty_index;
            empty_index++;
            accessing_functions[index] = function_name;
            this->access_type[index] = access_type;
            access_count[index] = 0;
            function_invocations[index] = 1;
            accessor_id[index] = function_id;
        }
    }
    
    if (increment){
        /// Add this access to the log
        //access_log.append("#");
        access_log.append(function_name);
        access_log.append(" ");
        char temp[1024];
        sprintf(temp,"%d",function_id);
        access_log.append(temp);
        access_log.append(" ");
        access_log.push_back(access_type);
        access_log.push_back('\n');
    }
    
    
}

int address_node::get_first_empty_index(){
    for (int i = 0; i < MAX_FUNCTIONS; i++){
        if (access_count[i] == -1) return i;
    }

}

void address_node::print_accessing_functions(){
    cout<<"FUNCTIONS ACCESSING ("<<address<<"):"<<endl;
    for (int i = 0; i < MAX_FUNCTIONS; i++){
        if (access_count[i] == -1) break;
        cout<<accessing_functions[i]<<" (";
        if (access_type[i] == STACKREAD) cout<<"STACKREAD";
        else if (access_type[i] == STACKWRITE) cout<<"STACKWRITE";
        else if (access_type[i] == HEAPREAD) cout<<"HEAPREAD";
        else if (access_type[i] == HEAPWRITE) cout<<"HEAPWRITE";
        cout<<")("<<access_count[i]<<")"<<endl;
    }
    cout<<endl;

}
void address_node::print_accessing_functions(string options){
    bool print_or_not = false;

    for (int i = 0; i < MAX_FUNCTIONS; i++){
        if (access_count[i] == -1) break;

        // Depending on options specified
        if (options.find("nostackread") != string::npos && access_type[i] == STACKREAD) continue;
        if (options.find("nostackwrite") != string::npos && access_type[i] == STACKWRITE) continue;
        if (options.find("noheapread") != string::npos && access_type[i] == HEAPREAD) continue;
        if (options.find("noheapwrite") != string::npos && access_type[i] == HEAPWRITE) continue;
        if (options.find("noprint") != string::npos && accessing_functions[i].find("print") != string::npos) continue;

        // by this point, should print something. If this is the first time, add the "FUNCTIONS ACCESSING" header
        if (print_or_not == false){
            print_or_not = true;
            cout<<"FUNCTIONS ACCESSING ("<<address<<"):"<<endl;
        }

        cout<<accessing_functions[i]<<" (";
        if (access_type[i] == STACKREAD) cout<<"STACKREAD";
        else if (access_type[i] == STACKWRITE) cout<<"STACKWRITE";
        else if (access_type[i] == HEAPREAD) cout<<"HEAPREAD";
        else if (access_type[i] == HEAPWRITE) cout<<"HEAPWRITE";
        cout<<")("<<access_count[i]<<")("<<function_invocations[i]<<")"<<endl;

    }
    if (print_or_not == true) cout<<endl;

}

int count_char (string str, char c){
    int count = 0;
    for (int i = 0; i < str.length(); i++){
        if (str[i] == c) count++;
    }
    return count;
}

/*  Given a function name, determine whether this function is parallelizeable.
    Go through the list of addresses, see if there are conflicts with other funcitons

    heap conflicts (where there is at least 1 heap write and other functions also access)
    always means not parallelizable.

    stack conflicts must be checked if the variable is local to the higher level function that calls the
    2 functions in conflict.
    ex: (at a given address)
    main>   (STACKREAD)
    main>add_64   (STACKREAD)
    main>sub_64   (STACKWRITE)

    The add/sub are NOT parallelizable because they read/write to a variable that
    is local to main, which calls sub and add.

    ex: (at a given address)
    main>add_64   (STACKREAD)
    main>sub_64   (STACKWRITE)

    The above IS parallelizable because the variable is local to each of the functions!
*/
bool parallelizable (address_node* list, const char* function_name, const char* file){
	
	ofstream outputFile;
	string file_name = file;
	file_name.append("_");
	file_name.append(function_name);
	file_name.append("_parallelize_analysis.txt");
    outputFile.open(file_name.c_str());
	
	
    // The function name string we will be looking at. Append > to it. (just the convention i chose)
    string function = function_name;
    function.append(">");
    // Keep list of the conflicting functions
    string heapreadconflicts;
    string heapwriteconflicts;
    string stackreadconflicts;
    string stackwriteconflicts;
    // pointer to traverse the list
    address_node* curr = list;
    // Self explanatory variables
    bool heap_conflict_with_other = false;
    bool heap_conflict_with_self = false;
    bool stack_conflict_with_other = false;
    bool stack_conflict_with_self = false;
    bool heap_dependency_conflict = false;
    bool stack_dependency_conflict = false;

    outputFile<<"------------- Checking for function: "<<function_name<<" -------------"<<endl;
/*
    /// DEPENDENCY CHECKS
    // Check for write/read dependency (ie a previous instance has written to addr
    // and now another instance is trying to read from addr). This may not be picked up by
    // the check for conflicts with self, because the number of previous writes may be 1
    // This is mianly an issue for functions with changing access patterns (ie one invocation
    // might only write, another invocation might only read, etc).
    while (curr != NULL){
        int stringindex = 0;
        int fieldbegin,fieldend;
        bool previouslywritten = false;
        string previousfunctionid;
        if (curr->address == "80032e60") outputFile<<curr->access_log<<endl;
        while(1){
            // If reached end of string, break;
            if (stringindex >= curr->access_log.length()) break;
            
            // Check for access by the function. If no more accessed by the function, break
            stringindex = curr->access_log.find(function_name,stringindex);
            if (stringindex == -1) break;
            
            // get the function id
            fieldbegin = curr->access_log.find(' ',stringindex);
            fieldend = curr->access_log.find(' ',fieldbegin+1);
            string function_id = curr->access_log.substr(fieldbegin+1,fieldend-fieldbegin-1);
            
            // get the access type
            char access_type = curr->access_log[fieldend+1];
            
            // Increment the stringindex for the next scan
            stringindex++;
            
            // lock in the first write. If there is any read from now on coming from a different invocation
            // of the function (different function_id) then we know there is a dependency issue
            if ((access_type == HEAPWRITE || access_type == STACKWRITE) && !previouslywritten){ 
                previouslywritten = true;
                previousfunctionid = function_id;
                continue;
            }
            
            // Check for the dependency
            if ((access_type == HEAPREAD || access_type == STACKREAD) && previouslywritten && previousfunctionid != function_id){
                outputFile<<"WTFFFF"<<endl;
                dependency_conflict = true;
            }
        }
        curr=curr->next;
    }
*/
    
    /// HEAP CHECKS
    // Sweep through all the addresses
    while (curr != NULL){
        bool function_has_accessed_this_addr = false;
        bool func_heap_read = false; // Has the specified function read from the global var?
        bool func_heap_write = false; // Has the specified function written to the global var?
        bool other_func_heap_read = false; // Has another function rea dfrom the global var?
        bool other_func_heap_write = false; // Has another function written to the global var?
        bool dependency_conflict_at_addr = false;
        int func_id = 0;
        int func_heap_writes = 0; // Keep track of how many times the specified function writes to the global var. if this is more than 1, conflict with itself

        // If this address is not a heap variable, skip it
        if (curr->access_type[0] != HEAPREAD && curr->access_type[0] != HEAPWRITE){
            curr = curr->next;
            continue;
        }

        /// dependency conflicts with heap
        int stringindex = 0;
        int fieldbegin,fieldend;
        bool previouslywritten = false;
        string previousfunctionid;
        while(1){
            // If reached end of string, break;
            if (stringindex >= curr->access_log.length()) break;
            
            // Check for access by the function. If no more accessed by the function, break
            stringindex = curr->access_log.find(function_name,stringindex);
            if (stringindex == -1) break;
            
            // get the function id
            fieldbegin = curr->access_log.find(' ',stringindex);
            fieldend = curr->access_log.find(' ',fieldbegin+1);
            string function_id = curr->access_log.substr(fieldbegin+1,fieldend-fieldbegin-1);
            
            // get the access type
            char access_type = curr->access_log[fieldend+1];
            
            // Increment the stringindex for the next scan
            stringindex++;
            
            // lock in the first write. If there is any read from now on coming from a different invocation
            // of the function (different function_id) then we know there is a dependency issue
            if ((access_type == HEAPWRITE) && !previouslywritten){ 
                previouslywritten = true;
                previousfunctionid = function_id;
                continue;
            }
            
            // Check for the dependency
            if ((access_type == HEAPREAD) && previouslywritten && previousfunctionid != function_id && dependency_conflict_at_addr == false){
                outputFile<<"Heap dependency conflict at: "<<curr->address<<endl;
                //outputFile<<"Access log: "<<curr->access_log<<endl;
                dependency_conflict_at_addr = true;
                heap_dependency_conflict = true;
            }
        }
        
        /// Other conflicts with heap
        // Sweep through all the function accesses to this address
        for (int i = 0; i < MAX_FUNCTIONS && curr->access_count[i] != -1; i++){

            // specified function reads from heap
            if (curr->accessing_functions[i].find(function) != string::npos &&
                curr->access_type[i] == HEAPREAD) func_heap_read = true;
            // specified function writes to heap (add to func_heap_writes)
            if (curr->accessing_functions[i].find(function) != string::npos &&
                curr->access_type[i] == HEAPWRITE){ 
                func_heap_write = true; 
                //func_heap_writes = func_heap_writes + curr->function_invocations[i];
                
                
                if (func_id == 0){
                    func_id = curr->accessor_id[i];
                    func_heap_writes = func_heap_writes + curr->function_invocations[i];
                } else {
                    if (curr->function_invocations[i] >= 2){
                        func_heap_writes = func_heap_writes + curr->function_invocations[i];
                    } else if (curr->accessor_id[i] != func_id) {
                        func_heap_writes++;
                    }
                
                }
                
                
            }
            // another function reads from heap
            if (curr->accessing_functions[i].find(function) == string::npos &&
				curr->accessing_functions[i] != "main>" &&
                curr->access_type[i] == HEAPREAD) other_func_heap_read = true;
            // another function writes to heap
            if (curr->accessing_functions[i].find(function) == string::npos &&
				curr->accessing_functions[i] != "main>" &&
                curr->access_type[i] == HEAPWRITE) other_func_heap_write = true;
        }

        // Check for conflicts with others due to global variable (heap) issues
        if ((func_heap_read || func_heap_write) && other_func_heap_write){
            outputFile<<"Heap conflict with other at: "<<curr->address<<endl<<flush;
	    //outputFile<<"Access log: "<<curr->access_log<<endl;
            heap_conflict_with_other = true;
        }
        if (func_heap_write && (other_func_heap_read || other_func_heap_write)){
            outputFile<<"Heap conflict with other at: "<<curr->address<<endl<<flush;
	    //outputFile<<"Access log: "<<curr->access_log<<endl;
            heap_conflict_with_other = true;
        }
        // Check for conflict with self due to global variable (heap) issues
        //if the specified function writes to heap addr multiple times, conflict with self
        if (func_heap_writes > 1){ 
            outputFile<<"Heap conflict with self at: "<<curr->address<<endl<<flush;
	    //outputFile<<"Access log: "<<curr->access_log<<endl;
            heap_conflict_with_self = true;
        } 

        curr = curr->next;
    }

    /// STACK CHECKS
    // Sweep through all the addresses
    curr = list;
    while (curr != NULL){
        
        bool dependency_conflict_at_addr = false;

        // If this address is not a stack variable, skip it
        if (curr->access_type[0] != STACKREAD && curr->access_type[0] != STACKWRITE){
            curr = curr->next;
            continue;
        }

        // Find the localities we are concerned about
        // Ex: we are checking for parallizableness of add_32
        // main>
        // main>add_32
        // main>sub_32
        // Here, only 1 locality is important: "main>"
        //
        // Ex: we are checking for parallelizableness of funD
        // main>funA>
        // main>funB>funC>
        // main>funB>funC>funD
        // main>funB>funC>funE
        // main>funG>funC>
        // main>funG>funC>funD
        // main>funG>funC>funE
        // Here, 2 localities are important: "main>funB>funC>" and "main>funG>funC>"
        string localities [MAX_FUNCTIONS*4];
        int localities_count = 0;

        // First find all the localities
        for (int i = 0; i < MAX_FUNCTIONS && curr->access_count[i] != -1; i++){

            // If this is an access by the fnuction in question, look for the sub strings
            // (the possible localities) that may exist
            // ex: if checking for parallelizableness of funD and we encounter
            //  main>funA>funB>funD
            //
            // here, we need to look for "main>" and "main>funA>" and "main>funA>funB>"
            // (see if there are accesses by those call paths)
            if (curr->accessing_functions[i].find(function) != string::npos){
                for (int j = 0; j < MAX_FUNCTIONS && curr->access_count[j] != -1; j++){
                    // Ignore itself (as we again sweep from the beginning, we will run into the call path itself)
                    if (curr->accessing_functions[i] == curr->accessing_functions[j]) continue;

                    // if access is a sub string, it is a locality
                    if (curr->accessing_functions[i].find(curr->accessing_functions[j]) != string::npos){
                        bool locality_already_found = false;
                        // See if we've already found this locality
                        for (int k = 0; k < localities_count; k++)
                            if (localities[k] == curr->accessing_functions[k]){ 
                                locality_already_found = true;
                                break;
                            }
                            
                        if (locality_already_found) continue;
                            
                        // Since we haven't found the locality,add it and increase the count
                        localities[localities_count] = curr->accessing_functions[j];
                        localities_count++;
                    }
                }
            }
        }

        // if localities_count = 0, then this variable is purely local to the function in question.
        if (localities_count == 0){
            curr = curr->next;
            continue;
        }


        // Sweep through the function accesses again, this time check for other functions accessing the localities
        for (int j = 0; j < localities_count; j++){

            // due to some noise in gxemul, skip a few local variables of main.
            /*if ((curr->address == "a0007ed8" ||
                curr->address == "a0007edc" ||
                curr->address == "a0007ee0" ||
                curr->address == "a0007ee4" ||
                curr->address == "a0007ee8" ||
                curr->address == "a0007ef4" ||
                curr->address == "a0007ef0" ||
                curr->address == "a0007eec") &&
                localities[j] == "main>"){
                continue;
            }*/
            
            // If this variable is local to the function itself, we don't care!
            if (localities[j].find(function_name) != string::npos) continue;

            /// Dependency checks for stack
            int stringindex = 0;
            int fieldbegin,fieldend;
            bool previouslywritten = false;
            string previousfunctionid;
            while(1){
                // If we've already found a conflict, no point in looking for more.
                if (stack_dependency_conflict) break;
                
                // If reached end of string, break;
                if (stringindex >= curr->access_log.length()) break;
                
                // Find an access to a variable from the locality we are checking. If none, break;
                stringindex = curr->access_log.find(localities[j],stringindex);
                if (stringindex == -1) break;
                
                // get the function id
                fieldbegin = curr->access_log.find(' ',stringindex);
                fieldend = curr->access_log.find(' ',fieldbegin+1);
                string function_id = curr->access_log.substr(fieldbegin+1,fieldend-fieldbegin-1);
                
                // get the access type
                char access_type = curr->access_log[fieldend+1];
                
                // Increment the stringindex for the next scan
                stringindex++;
                
                // Check if this access was by the function we are checking. If not, go to the next access.
                if (curr->access_log.substr(stringindex,fieldbegin-stringindex).find(function_name) == string::npos) continue;
                
                // Check if this access was in the locality currently being checked
                // If the call path does not include the localities, skip this access
                //if (curr->access_log.substr(stringindex,fieldbegin-stringindex).find(localities[j]) == string::npos) continue;
                
                // lock in the first write. If there is any read from now on coming from a different invocation
                // of the function (different function_id) then we know there is a dependency issue
                if (access_type == STACKWRITE && !previouslywritten){ 
                    previouslywritten = true;
                    previousfunctionid = function_id;
                    continue;
                }
                
                // Check for the dependency
                if (access_type == STACKREAD && previouslywritten && previousfunctionid != function_id && dependency_conflict_at_addr == false){
                    outputFile<<"Stack dependency conflict at: "<<curr->address<<endl;
                    //outputFile<<"Access log: "<<curr->access_log<<endl;
                    stack_dependency_conflict = true;
                    dependency_conflict_at_addr = true;
                }
            }

            /// Other conflicts for stack
            bool local_to_lower_depth_and_func_reads = false;
            bool local_to_lower_depth_and_func_writes = false;
            bool local_to_lower_depth_and_other_reads = false;
            bool local_to_lower_depth_and_other_writes = false;

            int func_stack_writes = 0;
            int func_id = 0;

            // ignore main locality...
            //if (localities[j].find("main") != string::npos) continue;

            for (int i = 0; i < MAX_FUNCTIONS && curr->access_count[i] != -1; i++){

                // specified function reads from variable from locality j
                if (curr->accessing_functions[i].find(function) != string::npos &&
                    curr->access_type[i] == STACKREAD &&
                    curr->accessing_functions[i].find(localities[j]) != string::npos){
                    local_to_lower_depth_and_func_reads = true;
                }
                // specified function writes to variable from locality j
                if (curr->accessing_functions[i].find(function) != string::npos &&
                    curr->access_type[i] == STACKWRITE &&
                    curr->accessing_functions[i].find(localities[j]) != string::npos){
                    local_to_lower_depth_and_func_writes = true;
                    // A bit of a hack to check for number of UNIQUE stack writes.
                    // If there are writes from multiple call paths, need to see if
                    // these multiple call paths are from different invocations or the same.
                    // ie. If they came from the same then we shouldn't double count them.
                    // But because we only have the function_id of the LATEST call path
                    // that accessed it, we rely on the fact that if function_invocations[i]
                    // is >= 2, then automatically there is a conflict.
                    // If it is 1, THEN it is important to check for which invocation it is from.
                    if (func_id == 0){
                        func_id = curr->accessor_id[i];
                        func_stack_writes = func_stack_writes + curr->function_invocations[i];
                    } else {
                        if (curr->function_invocations[i] >= 2){
                            func_stack_writes = func_stack_writes + curr->function_invocations[i];
                        } else if (curr->accessor_id[i] != func_id) {
                            func_stack_writes++;
                        }
                    
                    }
                    
                    // Hack... only check for >= 2. This can fail for checking a function
                    // with changing access patterns (each call of the function has a 
                    // different access pattern depending on inputs)
                    //if (curr->function_invocations[i] >= 2) func_stack_writes = 10;
                    
                    
                    //func_stack_writes = func_stack_writes + curr->function_invocations[i];
                    //outputFile<<"CURRENT FUNCTION INVOCATIONS: "<<curr->function_invocations[i]<<endl;

                    //outputFile<<curr->accessing_functions[i]<<endl;
                }
                // another function reads from variable from locality j
                // The last condition is to ignore reads by the parent function.
                if (curr->accessing_functions[i].find(function) == string::npos &&
                    curr->access_type[i] == STACKREAD &&
                    curr->accessing_functions[i].find(localities[j]) != string::npos &&
                    curr->accessing_functions[i] != localities[j]){
                    local_to_lower_depth_and_other_reads = true;
                }
                // another function writes to variable from locality j
                // The last condition is to ignore writes by the parent function.
                if (curr->accessing_functions[i].find(function) == string::npos &&
                    curr->access_type[i] == STACKWRITE &&
                    curr->accessing_functions[i].find(localities[j]) != string::npos &&
                    curr->accessing_functions[i] != localities[j]){
                    local_to_lower_depth_and_other_writes = true;
                }
            }
            // Check for conflicts with other functions
            if (local_to_lower_depth_and_other_writes &&
                (local_to_lower_depth_and_func_writes || local_to_lower_depth_and_func_reads)){
                stack_conflict_with_other = true;
                outputFile<<"stack conflict with other at: "<<curr->address<<endl;
                //outputFile<<"Access log: "<<curr->access_log<<endl;
            }
            if (local_to_lower_depth_and_func_writes &&
                (local_to_lower_depth_and_other_writes || local_to_lower_depth_and_other_reads)){
                stack_conflict_with_other = true;
                outputFile<<"stack conflict with other at: "<<curr->address<<endl;
		//outputFile<<"Access log: "<<curr->access_log<<endl;
            }
            // Check for conflicts with self
            if (func_stack_writes > 1){ 
                stack_conflict_with_self = true;
                outputFile<<func_stack_writes<<" ";
                outputFile<<"stack conflict with self at: "<<curr->address<<endl;
		//outputFile<<"Access log: "<<curr->access_log<<endl;
            }
        }
        
        

        curr = curr->next;
    }


    outputFile<<"stack conflict with other: "<<stack_conflict_with_other<<endl<<flush;
    outputFile<<"stack conflict with self: "<<stack_conflict_with_self<<endl<<flush;
    outputFile<<"heap conflict with other: "<<heap_conflict_with_other<<endl<<flush;
    outputFile<<"heap conflict with self: "<<heap_conflict_with_self<<endl<<flush;
    outputFile<<"heap dependency conflict with self: "<<heap_dependency_conflict<<endl<<flush;
    outputFile<<"stack dependency conflict with self: "<<stack_dependency_conflict<<endl<<flush;
    
    outputFile<<"----------------------- DONE FOR: "<<function_name<<" -----------------------"<<endl;
	
	
	
	outputFile.close();
	
    return !stack_conflict_with_other && !stack_conflict_with_self && !heap_conflict_with_other 
            && !heap_conflict_with_self && !heap_dependency_conflict && !stack_dependency_conflict;
}



