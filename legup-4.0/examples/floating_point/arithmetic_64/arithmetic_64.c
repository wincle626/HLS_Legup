#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {

    double array[3];
    array[0] = 1.1;
    array[1] = 2.2;
    array[2] = 3.3;

    double array2[2] = {1.5, 2.5};

    double a = -4.2;
    double b = 7.4;


    double temp = array[0] + array2[0] + a; //-1.6
    double temp2 = array[1]+array[2]-array2[1]; //3
    double temp3 = b*4 + temp; //28
    temp = (temp+array2[1])/temp2; //0.3
    double temp4 = temp2*temp;  //0.9
    
    if (((temp4 - 0.9)<1e-6)&&((temp4 - 0.9)>-1e-6)){
        printf("Pass");
        return 123;
    }
    else {
        printf("Fail");
        return -1;
    }
}



