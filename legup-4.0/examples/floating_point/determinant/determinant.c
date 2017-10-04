#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)
typedef double Number;

#include <stdio.h>

#pragma map generate_hw 7
Number determinant2x2(double *Adata)
{
    Number a,b,c,d;
    a = Adata[0]; b = Adata[1];
    c = Adata[2]; d = Adata[3];
    return a*d - b*c;
}
#pragma map generate_hw 7
Number determinant3x3(double *Adata)
{
    Number a,b,c,d,e,f,g,h,i;
    a = Adata[0]; b = Adata[1]; c = Adata[2];
    d = Adata[3]; e = Adata[4]; f = Adata[5];
    g = Adata[6]; h = Adata[7]; i = Adata[8];
    return a*e*f - b*d*i + b*f*g + c*d*h - c*e*g;
}

int main() {
    double matrix_2x2[4];
    matrix_2x2[0] = 5;
    matrix_2x2[1] = 13;
    matrix_2x2[2] = 9;
    matrix_2x2[3] = 47;
    
    double matrix_3x3[9] = {4, 8, 3, 9, 3, 7, 1, 9, 4};
    
    double det_2x2 = determinant2x2(matrix_2x2);
    double det_3x3 = determinant3x3(matrix_3x3);
    double zero = 0;
    if (det_2x2 < zero)
        det_2x2 = -det_2x2;
    if (det_3x3 < zero)
        det_3x3 = -det_3x3;

    double diff;
    if (det_2x2 > det_3x3)
        diff = det_2x2-det_3x3;
    else
        diff = det_3x3-det_2x2;

    int counter;
    for (counter = 0; counter < diff; counter++)
        ;

    printf("%d\n", counter);

    return counter;

}



