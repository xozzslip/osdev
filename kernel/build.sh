set -e
set -x
BIN=/home/khasan/code/osdev/cross-compiler/i386-elf/bin

nasm -f bin boot.asm -o boot.bin
# nasm -f bin kernel_entry.asm -o kernel.bin
nasm -f elf kernel_entry.asm -o kernel_entry.o
$BIN/i386-elf-gcc -ffreestanding -c kernel.c -o kernel.o
$BIN/i386-elf-ld -o kernel.bin -Ttext 0x1000 --oformat binary kernel_entry.o kernel.o
cat boot.bin kernel.bin > out.bin

qemu-system-x86_64 -drive format=raw,file=out.bin