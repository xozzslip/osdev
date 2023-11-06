set -e
set -x

i386-elf-gcc -ffreestanding -c hello.c -o hello.o
# i386-elf-objdump -M intel -d hello.o
i386-elf-ld -o hello.bin -Ttext 0x0 --oformat binary hello.o
ndisasm -b 32 hello.bin
xxd hello.bin


