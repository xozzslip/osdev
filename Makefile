CC = i386-elf-gcc
LD = i386-elf-ld
GDB = i386-elf-gdb
QEMU = qemu-system-i386 -m 128M -drive format=raw,file=build/disk.img

$(shell find src -type d -exec sh -c 'mkdir -p build/$$(echo "{}" | sed "s|^src||")' \;)
KERNEL_C = $(shell find src/kernel -type f -name '*.c')
KERNEL_ASM = $(shell find src/kernel -type f -name '*.asm')
KERNEL_H = $(shell find src/kernel -type f -name '*.h')
KERNEL_H += $(shell find src/include -type f -name '*.h')
KERNEL_ENTRY_OBJ = build/kernel/entry.o
KERNEL_OBJ = $(patsubst src/%.c,build/%.o,$(KERNEL_C))
KERNEL_OBJ += $(patsubst src/%.asm,build/%.o,$(KERNEL_ASM))
KERNEL_LD = src/kernel/linker.ld
BOOT_ASM = $(shell find src/boot -type f -name '*.asm')
BOOT_OBJ = $(patsubst src/%.c,build/%.o,$(BOOT_ASM))

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

build/disk.img: build/boot/boot.bin build/kernel.bin scripts/flush_os_to_disk.py
	python scripts/flush_os_to_disk.py build/disk.img build/boot/boot.bin build/kernel.bin

.PHONY: build
build: build/disk.img build/kernel.elf

.PHONY: disasm
disasm: build/kernel.elf
	objdump -d -M intel $<

build/kernel.bin: $(KERNEL_LD) $(KERNEL_OBJ)
	$(LD) $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ) -o $@ -T $(KERNEL_LD) --oformat binary

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
build/kernel.elf: $(KERNEL_LD) $(KERNEL_OBJ)
	$(LD) $(KERNEL_ENTRY_OBJ) $(KERNEL_OBJ) -o $@ -T $(KERNEL_LD) -Map=build/linker.map

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
