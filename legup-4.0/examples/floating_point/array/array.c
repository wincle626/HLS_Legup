#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {
    int return_val = 0;

    float exp1 = 1.1;
    float exp2 = 2.2;
    float exp3 = 3.3;
    float exp4 = 4.4;
    float array[3] = {1.1, 2.2, 3.3};
    printf("ARRAY FLT: %x, %x, %x\n", FLT2HEX(array[0]), FLT2HEX(array[1]), FLT2HEX(array[2]));
    printf("EXPEC FLT: %x, %x, %x\n", FLT2HEX(exp1), FLT2HEX(exp2), FLT2HEX(exp3));

    if (exp1 == array[0])
        return_val++;
    if (exp2 == array[1])
        return_val++;
    if (exp3 == array[2])
        return_val++;

    double dexp1 = 1.1;
    double dexp2 = 2.2;
    double dexp3 = 3.3;
    double dexp4 = 4.4;
    double darray[3] = {1.1, 2.2, 3.3};
    printf("ARRAY DBL: %llx, %llx, %llx\n", DBL2HEX(darray[0]), DBL2HEX(darray[1]), DBL2HEX(darray[2]));
    printf("EXPEC DBL: %llx, %llx, %llx\n", DBL2HEX(dexp1), DBL2HEX(dexp2), DBL2HEX(dexp3));

    if (dexp1 == darray[0])
        return_val++;
    if (dexp2 == darray[1])
        return_val++;
    if (dexp3 == darray[2])
        return_val++;

    float check = array[0] + array[1];
    printf("%x, %x\n", FLT2HEX(check), FLT2HEX(array[2]));
    if (((check - array[2]) < 1e-6) || ((array[2] - check) < 1e-6))
        return_val++;
    double dcheck = darray[0] + darray[1];
    printf("%x, %x\n", FLT2HEX(dcheck), FLT2HEX(darray[2]));
    if (((dcheck - darray[2]) < 1e-6) || ((darray[2] - dcheck) < 1e-6))
        return_val++;

    array[0] = 4.4;
    if (exp4 == array[0])
        return_val++;

    darray[0] = 4.4;
    if (dexp4 == darray[0])
        return_val++;


    return return_val;
}



