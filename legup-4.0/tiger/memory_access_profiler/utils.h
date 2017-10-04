#include <string>
#define MAX_FUNCTIONS 128
#define STACKREAD 'a'
#define STACKWRITE 'b'
#define HEAPREAD 'c'
#define HEAPWRITE 'd'

using namespace std;

class register_node {
public:
	//string reg_name;
	string written_by; // which fnuction path wrote to this register last?
	string associated_address; // If the data from an address was loaded to this reg, keep that address.
	//register_node* next;
	int func_id;
};

class address_node {
    public:
    int different_functions_accessed;
    address_node();
    string address;
    string accessing_functions[MAX_FUNCTIONS];
    char access_type[MAX_FUNCTIONS];
    int access_count[MAX_FUNCTIONS];
    int function_invocations[MAX_FUNCTIONS];// how many invocations of the function access the address?
    address_node* next;
   //bool newinvocation[MAX_FUNCTIONS];
    
    string access_log;
    int empty_index;
    
    //hold the ID of the accesser
    int accessor_id[MAX_FUNCTIONS];

    void print_accessing_functions();
    // put in options desired. ex "nostack" to not print out any stack info
    // combine options by adding them ex "nostacknoread"
    void print_accessing_functions(string options);
    void add_access(string accessing_function, char access_type,int function_id, bool increment);
    int find_function (string accessing_function, char access_type);
    int get_first_empty_index();
    bool previously_accessed (string function_name);
};
bool parallelizable (address_node* list, const char* function, const char* file);
unsigned int hexStrToInt (const char* str);
address_node* find_address(address_node* list, const char* address);
int count_char (string str, char c);
int find_conflicts (address_node* list, const char* function1, const char* function2, int call_depth, string options);

