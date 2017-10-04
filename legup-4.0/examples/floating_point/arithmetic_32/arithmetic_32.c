#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {

    float array[3];
    array[0] = 1.1;
    array[1] = 2.2;
    array[2] = 3.3;

    float array2[2] = {1.5, 2.5};

    float a = -4.2;
    float b = 7.4;


    float temp = array[0] + array2[0] + a; //-1.6
    float temp2 = array[1]+array[2]-array2[1]; //3
    float temp3 = b*4 + temp; //28
    temp = (temp+array2[1])/temp2; //0.3
    float temp4 = temp2*temp;  //0.9
    
    if (((temp4 - 0.9)<1e-6)&&((temp4 - 0.9)>-1e-6)){
        printf("Pass");
        return 123;
    }
    else {
        printf("Fail");
        return -1;
    }
}



