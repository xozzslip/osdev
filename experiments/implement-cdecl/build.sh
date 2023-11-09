set -e
set -x

/usr/bin/x86_64-pc-linux-gnu-gcc-13.2.1 -m32 -O0 -c local_var.c -o local_var.o
nasm -f elf32 func.asm -o func.o
/usr/bin/x86_64-pc-linux-gnu-gcc-13.2.1 -m32 local_var.o func.o -o a.out
objdump -M intel -d local_var.o
./a.out