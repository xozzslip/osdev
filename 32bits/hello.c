int main() {
    int a = 5;
    int b = 10;

    __asm__ (
        "push %rip\n"
        "pop %rax\n"
    );

    // __asm__ (
    //     "mov $5, %rax\n"
    //     "push %rax\n"
    //     "mov $8, %rax\n"
    //     "push %rax\n"
    //     "mov $15, %rax\n"
    //     "push %rax\n"
    //     "pop %rax\n"
    //     "pop %rax\n"
    //     "pop %rax\n"

    // );
    return a + b;
}