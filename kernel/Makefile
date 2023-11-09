CC = ~/code/osdev/cross-compiler/i386-elf/bin/gcc

boot.bin: boot.asm
	nasm -f bin boot.asm -o boot.bin

out.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > out.bin

kernel.bin: kernel.o kernel_entry.o
	$(CC) -o kernel.bin -Ttext 0x1000 --oformat binary kernel_entry.o kernel.o

kernel_entry.o: kernel_entry.asm
	nasm -f elf kernel_entry.asm -o kernel_entry.o

kernel.o: kernel.c
	$(CC) -ffreestanding -c kernel.c -o kernel.o

run: out.bin
	qemu-system-x86_64 -drive format=raw,file=out.bin
