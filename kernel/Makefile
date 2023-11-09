CC = ~/code/osdev/cross-compiler/i386-elf/bin/i386-elf-gcc
LD = ~/code/osdev/cross-compiler/i386-elf/bin/i386-elf-ld
GDB = ~/code/osdev/cross-compiler/i386-elf/bin/i386-elf-gdb


run: out.bin
	qemu-system-i386 -drive format=raw,file=out.bin

debug: out.bin kernel.elf
	qemu-system-i386 -s -drive format=raw,file=out.bin &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"

boot.bin: boot.asm
	nasm -f bin boot.asm -o boot.bin

out.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > out.bin

kernel.bin: kernel_entry.o kernel.o
	$(LD) $^ -o $@ -Ttext 0x1000 --oformat binary

# This is used for debugging
kernel.elf: kernel_entry.o kernel.o
	$(LD) $^ -o $@ -Ttext 0x1000

kernel_entry.o: kernel_entry.asm
	nasm -f elf kernel_entry.asm -o kernel_entry.o

kernel.o: kernel.c
	$(CC) -O0 -g -ffreestanding -c kernel.c -o kernel.o

clean:
	rm -rf out.bin boot.bin kernel.bin kernel_entry.o kernel.o
