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

.PHONY: run
run: $(BUILD_DIR)/os.bin
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/os.bin

.PHONY: build
build: $(BUILD_DIR)/os.bin

.PHONY: disasm
disasm: $(BUILD_DIR)/kernel.elf
	objdump -d -M intel $<

$(BUILD_DIR)/os.bin: $(BUILD_DIR)/boot/boot.bin $(BUILD_DIR)/kernel.bin
	cat $^ > $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel_entry.o $(OBJ_FILES)
	$(LD) $^ -o $@ -Ttext 0x7E00 --oformat binary

######
# This is used for debugging
$(BUILD_DIR)/kernel.elf: $(BUILD_DIR)/kernel_entry.o $(OBJ_FILES)
	$(LD) $^ -o $@ -Ttext 0x7E00

.PHONY: debug
debug: $(BUILD_DIR)/os.bin $(BUILD_DIR)/kernel.elf
	@( \
		qemu-system-i386 -s -drive format=raw,file=$(BUILD_DIR)/os.bin & \
		DAEMON_PID=$$!; \
		echo "Qemu process started as daemon PID $$DAEMON_PID"; \
		$(GDB) -ex "target remote localhost:1234" -ex "symbol-file $(BUILD_DIR)/kernel.elf" -x gdb_commands.txt; \
		kill -9 $$DAEMON_PID; \
		echo "Qemu process with PID $$DAEMON_PID was killed"; \
	)
#######

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(H_FILES)
	$(CC) -Werror -O0 -g -ffreestanding -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	nasm -f elf $< -o $@

$(BUILD_DIR)/%.bin: $(SRC_DIR)/%.asm
	nasm -f bin $< -o $@

clean:
	rm -rf $(BUILD_DIR) a.out
	ls

# Rules to run OS on the host
CC_HOST = /usr/bin/x86_64-pc-linux-gnu-gcc-13.2.1

host_run: kernel.c
	$(CC_HOST) -m32 -O0 -g kernel.c -o a.out
	./a.out

host_test: test.c
	$(CC_HOST) -m32 -O0 -g test.c -o a.out
	./a.out


#### Create disk image ####
$(BUILD_DIR)/disk.img: $(BUILD_DIR)/os.bin
	rm -rf $@
	dd if=/dev/zero of=$@ bs=1M count=34
	mformat -i $@ -F ::
	mmd -i $@ ::/boot
	mcopy -i $@ $< ::/boot
###########################
