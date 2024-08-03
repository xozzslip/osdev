CC = i386-elf-gcc
LD = i386-elf-ld
GDB = i386-elf-gdb
QEMU = qemu-system-i386 -m 128M -drive format=raw,file=build/disk.img

# Create all necessary folders in build dir
$(shell find src -type d -exec sh -c 'mkdir -p build/$$(echo "{}" | sed "s|^src||")' \;)

# Source files
COMMON_H = $(shell find src/include -type f -name '*.h')

KERNEL_H = $(shell find src/kernel -type f -name '*.h')
KERNEL_C = $(shell find src/kernel -type f -name '*.c')
KERNEL_ASM = $(shell find src/kernel -type f -name '*.asm')
KERNEL_OBJ = $(patsubst src/%.c,build/%.o,$(KERNEL_C))
KERNEL_OBJ += $(patsubst src/%.asm,build/%.o,$(KERNEL_ASM))
KERNEL_LD = src/kernel/linker.ld

BOOT_ASM = $(shell find src/boot -type f -name '*.asm')
BOOT_OBJ = $(patsubst src/%.c,build/%.o,$(BOOT_ASM))

USERSPACE_H = $(shell find src/usr -type f -name '*.h')
USERSPACE_C = $(shell find src/usr -type f -name '*.c')
USERSPACE_OBJ = $(patsubst src/%.c,build/%.o,$(USERSPACE_C))

# Run 32 bit machine with 128M of RAM
.PHONY: run
run: build/disk.img
	$(QEMU) -serial stdio

.PHONY: test
test: build/disk.img
	@( \
		rm -f build/kernel.log ; \
		$(QEMU) -display none -serial stdio >> build/kernel.log & \
		DAEMON_PID=$$! ; \
		echo "Qemu process started as daemon PID $$DAEMON_PID, listening serial port:" ; \
		python scripts/inspect_kernel_logs.py build/kernel.log ; \
		kill -9 $$DAEMON_PID; \
	)

build/disk.img: build/boot/boot.bin build/kernel.bin scripts/flush_os_to_disk.py build/usr/screen.elf
	python scripts/flush_os_to_disk.py --recreate build/disk.img build/boot/boot.bin build/kernel.bin build/usr/screen.elf

.PHONY: build
build: build/disk.img build/kernel.elf

.PHONY: disasm
disasm: build/kernel.elf
	objdump -d -M intel $<

build/kernel.bin: $(KERNEL_LD) $(KERNEL_OBJ)
	$(LD) $(KERNEL_OBJ) -o $@ -T $(KERNEL_LD) --oformat binary

build/%.o: src/%.c $(KERNEL_H) $(COMMON_H)
	$(CC) -Werror -g -O0 -ffreestanding -c $< -o $@

build/%.o: src/%.asm
	nasm -f elf $< -o $@

build/%.bin: src/%.asm
	nasm -f bin $< -o $@

build/usr/%.o: src/usr/%.c $(USERSPACE_H) $(COMMON_H)
	$(CC) -Werror -O0 -ffreestanding -c $< -o $@

build/usr/screen.elf: $(USERSPACE_OBJ)
	$(LD) -shared $(wildcard build/usr/screen/*.o) $(wildcard build/usr/libc/*.o) -o $@

.PHONY: clean
clean:
	rm -rf build/*

### Debugging ###
build/kernel.elf: $(KERNEL_LD) $(KERNEL_OBJ)
	$(LD) $(KERNEL_OBJ) -o $@ -T $(KERNEL_LD) -Map=build/linker.map

.PHONY: debug
debug: build/disk.img build/kernel.elf
	@( \
		$(QEMU) -display none -s & \
		DAEMON_PID=$$!; \
		echo "Qemu process started as daemon PID $$DAEMON_PID"; \
		$(GDB) -ex "target remote localhost:1234" -ex "symbol-file build/kernel.elf" -x gdb_commands.txt; \
		kill -9 $$DAEMON_PID; \
		echo "Qemu process with PID $$DAEMON_PID was killed"; \
	)
##################
