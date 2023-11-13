[org 0x7c00]
    ; reading from disk
    mov bx, 0x1000 ; memory address where data will be loaded to
    mov ah, 2 ; int 0x13 command
    mov al, 2 ; number of sectors to read (read two sectors)
    mov cl, 2 ; sector number  (1-17 dec.) enumerated from 1!
    mov ch, 0 ; track number is zero (hopefully that track has a few sectors)
    ; dl should be already set by BIOS, it is a number of disk
    mov dh, 0
    int 0x13
    cmp ah, 0 ; check for errros

    ; mov bx, 0x1000
    ; call print16_str

    ; switching to protected mode
    cli
    lgdt [gdt_descriptor_start]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp segment_code:protected_mode_entry_point

%include "boot/print16.asm"
%include "boot/gdt.asm"

[bits 32]
protected_mode_entry_point:
    mov ax, segment_data
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; print via memory mapped VGA
    mov ebx, 0xb8000 + 2 * (14 * 80 + 2)
    mov eax, 0
    mov al, 'X' ; write 'X' in the middle of screen
    mov ah, 0x0f ; white on black
    mov [ds:ebx], eax
    jmp 0x1000

times 510 - ($-$$) db 0
dw 0xaa55
