#include <stdio.h>

double d = 0.5;
float f = 1.5f;

void change_globals(double d2, float f2)
{
  d += d2;
  f += f2;
}

int main() {
    change_globals(0.25, 0.75f);

    printf("Globals: %lf %f\n", d, f);

    int result = ((d == 0.75) ? 1 : 0) + ((f == 2.25f) ? 1 : 0);

    if (result != 2) {
        printf("FAIL: Converted: %lf %f", d, f);
        printf("FAIL: Expected: %lf %f", 0.75, 2.25f);
    }

    return result;
}



