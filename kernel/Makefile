$(shell mkdir -p build build/drivers build/boot build/interrupts)
C_FILES = $(wildcard src/*.c src/drivers/*.c src/interrupts/*.c)
H_FILES = $(wildcard src/*.h src/drivers/*.h src/interrupts/*.h)
ASM_FILES = $(wildcard src/interrupts/*.asm)
BUILD_DIR = build
SRC_DIR = src
OBJ_FILES = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(C_FILES))
OBJ_FILES += $(patsubst src/%.asm,$(BUILD_DIR)/%.o,$(ASM_FILES))

CC = i386-elf-gcc
LD = i386-elf-ld
GDB = i386-elf-gdb

$(BUILD_DIR)/disk.img: $(BUILD_DIR)/boot/boot.bin $(BUILD_DIR)/kernel.bin
	python scripts/flush_os_to_disk.py $@ $^

.PHONY: run
run: $(BUILD_DIR)/disk.img
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/disk.img

.PHONY: build
build: $(BUILD_DIR)/disk.img

.PHONY: disasm
disasm: $(BUILD_DIR)/kernel.elf
	objdump -d -M intel $<

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel_entry.o $(OBJ_FILES)
	$(LD) $^ -o $@ -Ttext 0x7E00 --oformat binary

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(H_FILES)
	$(CC) -Werror -O0 -g -ffreestanding -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	nasm -f elf $< -o $@

$(BUILD_DIR)/%.bin: $(SRC_DIR)/%.asm
	nasm -f bin $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

### Debugging ###
$(BUILD_DIR)/kernel.elf: $(BUILD_DIR)/kernel_entry.o $(OBJ_FILES)
	$(LD) $^ -o $@ -Ttext 0x7E00

.PHONY: debug
debug: $(BUILD_DIR)/disk.img $(BUILD_DIR)/kernel.elf
	@( \
		qemu-system-i386 -s -drive format=raw,file=$(BUILD_DIR)/disk.img & \
		DAEMON_PID=$$!; \
		echo "Qemu process started as daemon PID $$DAEMON_PID"; \
		$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $(BUILD_DIR)/kernel.elf" -x gdb_commands.txt; \
		kill -9 $$DAEMON_PID; \
		echo "Qemu process with PID $$DAEMON_PID was killed"; \
	)
##################
