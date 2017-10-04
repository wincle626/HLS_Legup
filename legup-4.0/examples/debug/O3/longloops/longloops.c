int gvar0;
int gvar1;

int foo(int a, int b) {
    int c, d, e;
    c = a * 2;
    d = b * 3;
    e = c + d;
    return e;
}

int main() {
    int var1;
    int var2;
    int sum;

    int i;
    int a = 1;
    int b = 2;
    sum = 0;

    for (i = 0; i < 1000; i++) {
        sum += foo(a, b);
        a += b;
    }

    for (i = 0; i < 100; i++) {
        sum -= i;
    }

    var1 = sum / 37;
    var2 = sum / 3;

    gvar0 = 1;
    gvar1 = 2;

    if (var1 > var2) {
        gvar0 = 0xAB;
        gvar1 = 0xCD;
    } else if (var2 > var1) {
        gvar0 = 0xEE;
        gvar1 = 0xFF;
    }

    int ret = var2 / var1 + gvar0 + gvar1;
    return ret;
}
