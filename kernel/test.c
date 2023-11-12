#include<stdio.h>

int main() {
    int a = 10;
    int b = 3333333;

    __asm__(
        "mov %1, %0\n\t"
        "add $32, %0\n\t"
        : "=r"(b)
        : "r"(a)
    );
    printf("b=%d\n", b);
}