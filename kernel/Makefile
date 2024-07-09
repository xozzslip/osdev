$(shell mkdir -p build build/drivers build/boot build/interrupts)
C_FILES = $(wildcard src/*.c src/drivers/*.c src/interrupts/*.c)
H_FILES = $(wildcard src/*.h src/drivers/*.h src/interrupts/*.h)
ASM_FILES = $(wildcard src/interrupts/*.asm)
OBJ_FILES = $(patsubst src/%.c,build/%.o,$(C_FILES))
OBJ_FILES += $(patsubst src/%.asm,build/%.o,$(ASM_FILES))

CC = i386-elf-gcc
LD = i386-elf-ld
GDB = i386-elf-gdb

build/disk.img: build/boot/boot.bin build/kernel.bin
	python scripts/flush_os_to_disk.py $@ $^

.PHONY: run
run: build/disk.img
	qemu-system-i386 -drive format=raw,file=build/disk.img

.PHONY: build
build: build/disk.img

.PHONY: disasm
disasm: build/kernel.elf
	objdump -d -M intel $<

build/kernel.bin: build/kernel_entry.o $(OBJ_FILES)
	$(LD) $^ -o $@ -Ttext 0x7E00 --oformat binary

build/%.o: src/%.c $(H_FILES)
	$(CC) -Werror -O0 -g -ffreestanding -c $< -o $@

build/%.o: src/%.asm
	nasm -f elf $< -o $@

build/%.bin: src/%.asm
	nasm -f bin $< -o $@

.PHONY: clean
clean:
	rm -rf build

### Debugging ###
build/kernel.elf: build/kernel_entry.o $(OBJ_FILES)
	$(LD) $^ -o $@ -Ttext 0x7E00

.PHONY: debug
debug: build/disk.img build/kernel.elf
	@( \
		qemu-system-i386 -s -drive format=raw,file=build/disk.img & \
		DAEMON_PID=$$!; \
		echo "Qemu process started as daemon PID $$DAEMON_PID"; \
		$(GDB) -ex "target remote localhost:1234" -ex "symbol-file build/kernel.elf" -x gdb_commands.txt; \
		kill -9 $$DAEMON_PID; \
		echo "Qemu process with PID $$DAEMON_PID was killed"; \
	)
##################
