set -e
set -x

gcc -g -O0 -c hello.c -o hello.o
objdump -M intel -d hello.o
gcc hello.o -o a.out
#./a.out
gdb ./a.out
