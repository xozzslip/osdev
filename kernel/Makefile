C_FILES = $(wildcard kernel/*.c drivers/*.c)
OBJ_FILES = $(patsubst %.c,%.o,${C_FILES})

CC = ~/code/osdev/cross-compiler/i386-elf/bin/i386-elf-gcc
LD = ~/code/osdev/cross-compiler/i386-elf/bin/i386-elf-ld
GDB = ~/code/osdev/cross-compiler/i386-elf/bin/i386-elf-gdb

run: os.bin
	qemu-system-i386 -drive format=raw,file=os.bin

os.bin: boot/boot.bin kernel/kernel.bin
	cat $^ > $@

kernel/kernel.bin: kernel/kernel_entry.o ${OBJ_FILES}
	$(LD) $^ -o $@ -Ttext 0x1000 --oformat binary

######
# This is used for debugging
kernel/kernel.elf: kernel/kernel_entry.o ${OBJ_FILES}
	$(LD) $^ -o $@ -Ttext 0x1000

debug: os.bin kernel/kernel.elf
	qemu-system-i386 -s -drive format=raw,file=os.bin &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file kernel/kernel.elf"
#######

%.o: %.c
	$(CC) -O0 -g -ffreestanding -c $< -o $@

%.o: %.asm
	nasm -f elf $< -o $@

%.bin: %.asm
	nasm -f bin $< -o $@

clean:
	rm -rf kernel/*.o kernel/*.bin

# Rules to run OS on the host
CC_HOST = /usr/bin/x86_64-pc-linux-gnu-gcc-13.2.1

run_host: kernel.c
	$(CC_HOST) -m32 -O0 -g kernel.c -o a.out
	./a.out

test_host: test.c
	$(CC_HOST) -m32 -O0 -g test.c -o a.out
	objdump -d -M intel ./a.out
	./a.out

