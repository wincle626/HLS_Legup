#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {
    float a = 5.9;
    float cmp_array[5] = {-6.2, 0, 3.8, 5.9, 9.9};

    int counter, eq, ne, grt, gre, lt, le;
    eq = 0; ne = 0; grt = 0; gre = 0; lt = 0; le = 0; 

    for (counter = 0; counter < 5; counter++){
        if (cmp_array[counter] == a)
            eq++;
        if (cmp_array[counter] != a)
            ne++;
        if (cmp_array[counter] < a)
            grt++;
        if (cmp_array[counter] <= a)
            gre++;
        if (cmp_array[counter] > a)
            lt++;
        if (cmp_array[counter] <= a)
            le++;
    }

    int return_val = 0;  //without the "= 0", there will be an error in Binding.cpp:620
    return_val = eq + ne*2 + grt*13 + gre*72 + lt*83 + le*111;

    printf("%d\n", return_val);

    return return_val;
}



