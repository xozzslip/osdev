#include <stdio.h>

int foo(int, int);
extern int bar(int, int);
int lbar(int, int);
int lfoo(int, int);

int main() {
    int a = 1;
    int b = 2;
    int c = lfoo(a, b);
    printf("lfoo=%d\n", c);

    int d = foo(a, b);
    printf("foo=%d\n", d);
}

int lfoo(int a, int b) {
    int c = 3;
    int d = lbar(a, b);
    return c + d;
}

int foo(int a, int b) {
    int c = 3;
    int d = bar(a, b);
    return c + d;
}

int mur(int a, int b) {
    int c = 4;
    printf("mur a=%d b=%d\n", a, b);
    return (a * a) + (b * c);
}

int lbar(int a, int b) {
    int c = 5;
    int d = 6;
    int e = mur(a, c) + mur(b, d);
    return e;
}