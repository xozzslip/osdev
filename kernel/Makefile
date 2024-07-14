$(shell mkdir -p build build/drivers build/boot build/interrupts)
C_FILES = $(wildcard src/*.c src/drivers/*.c src/interrupts/*.c)
H_FILES = $(wildcard src/*.h src/drivers/*.h src/interrupts/*.h)
ASM_FILES = $(wildcard src/interrupts/*.asm)
OBJ_FILES = $(patsubst src/%.c,build/%.o,$(C_FILES))
OBJ_FILES += $(patsubst src/%.asm,build/%.o,$(ASM_FILES))

CC = i386-elf-gcc
LD = i386-elf-ld
GDB = i386-elf-gdb
QEMU = qemu-system-i386 -m 128M -drive format=raw,file=build/disk.img

# Run 32 bit machine with 128M of RAM
.PHONY: run
run: build/disk.img
	$(QEMU)

.PHONY: test
test: build/disk.img
	@( \
		rm -f build/kernel.log ; \
		$(QEMU) -display none -serial stdio >> build/kernel.log & \
		DAEMON_PID=$$! ; \
		echo "Qemu process started as daemon PID $$DAEMON_PID" ; \
		python scripts/check_kernel_logs.py build/kernel.log ; \
		kill -9 $$DAEMON_PID; \
	)



build/disk.img: build/boot/boot.bin build/kernel.bin scripts/flush_os_to_disk.py
	python scripts/flush_os_to_disk.py build/disk.img build/boot/boot.bin build/kernel.bin

.PHONY: build
build: build/disk.img build/kernel.elf

.PHONY: disasm
disasm: build/kernel.elf
	objdump -d -M intel $<

build/kernel.bin: linker.ld build/kernel_entry.o $(OBJ_FILES)
	$(LD) build/kernel_entry.o $(OBJ_FILES) -o $@ -T linker.ld --oformat binary

build/%.o: src/%.c $(H_FILES)
	$(CC) -Werror -O0 -g -ffreestanding -c $< -o $@

build/%.o: src/%.asm
	nasm -f elf $< -o $@

build/%.bin: src/%.asm
	nasm -f bin $< -o $@

.PHONY: clean
clean:
	rm -rf build/*

### Debugging ###
build/kernel.elf: linker.ld build/kernel_entry.o $(OBJ_FILES)
	$(LD) build/kernel_entry.o $(OBJ_FILES) -o $@ -T linker.ld -Map=build/linker.map

.PHONY: debug
debug: build/disk.img build/kernel.elf
	@( \
		$(QEMU) -s & \
		DAEMON_PID=$$!; \
		echo "Qemu process started as daemon PID $$DAEMON_PID"; \
		$(GDB) -ex "target remote localhost:1234" -ex "symbol-file build/kernel.elf" -x gdb_commands.txt; \
		kill -9 $$DAEMON_PID; \
		echo "Qemu process with PID $$DAEMON_PID was killed"; \
	)
##################
