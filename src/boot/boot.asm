[org 0x7c00]
[bits 16]
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; reading from disk the rest of operating system
    mov bx, 0x7E00 ; memory address where data will be loaded to
    mov ah, 2 ; int 0x13 command
    mov al, 127 ; number of sectors to read (read 127 sectors (63.5 KiB)) we can change it easily up to about 400 KiB just by reading more sectors. But more we can load only in Protected mode because in real mode only 480KiB of memory are usable.
    mov cl, 2 ; sector number  (1-17 dec.) enumerated from 1!
    mov ch, 0 ; track number is zero (hopefully that track has a few sectors)
    ; dl should be already set by BIOS, it is a number of disk
    mov dh, 0
    int 0x13
    cmp ah, 0 ; check for errros
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; zero first usable area (0x0500-0x6500) of real mode address space
    mov ax, 0
    mov es, ax ; set segment to 0
    mov di, 0x0500 ; address from which we set zeroes
_zero_next_byte:
    mov byte [es:di], 0  ; store 0 ES:DI (0x1000:0x0050)
    inc di
    cmp di, 0x6500
    jne _zero_next_byte
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; reading memory map
    mov bx, 0 ; continuation value
    ; ES:DI Pointer to an Address Range Descriptor structure to fill in
    mov ax, 0
    mov es, ax ; set segment to 0
    mov di, 0x0500 ; address where to load data to
    mov edx, 0x534D4150 ; magic
_memprobe_next:
    mov ax, 0xE820 ; function code
    mov cx, 24 ; size of descriptor
    int 0x15
    add di, 24
    cmp bx, 0
    jne _memprobe_next
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; switching to protected mode
    cli
    lgdt [gdt_descriptor_start]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp segment_code:protected_mode_entry_point
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[bits 32]
protected_mode_entry_point:
    mov ax, segment_data
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Set stack to second MiB, so space for stack is 1 MiB
    mov esp, 0x1fffff
    ; print via memory mapped VGA
    ; mov ebx, 0xb8000 + 2 * (14 * 80 + 20)
    ; mov eax, 0
    ; mov al, 'C' ; write 'X' in the middle of screen
    ; mov ah, 0x0f ; white on black
    ; mov [ds:ebx], eax
    jmp 0x7E00

%include "src/boot/print16.asm"
%include "src/boot/gdt.asm"

times 510 - ($-$$) db 0
dw 0xaa55
