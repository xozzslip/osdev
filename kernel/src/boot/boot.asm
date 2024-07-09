[org 0x7c00]
[bits 16]
    ; reading from disk
    mov bx, 0x7E00 ; memory address where data will be loaded to
    mov ah, 2 ; int 0x13 command
    mov al, 127 ; number of sectors to read (read 127 sectors (63.5 KiB)) we can change it easily up to about 400 KiB just by reading more sectors. But more we can load only in Protected mode because in real mode only 480KiB of memory are usable.
    mov cl, 2 ; sector number  (1-17 dec.) enumerated from 1!
    mov ch, 0 ; track number is zero (hopefully that track has a few sectors)
    ; dl should be already set by BIOS, it is a number of disk
    mov dh, 0
    int 0x13
    cmp ah, 0 ; check for errros

    ; switching to protected mode
    cli
    lgdt [gdt_descriptor_start]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp segment_code:protected_mode_entry_point

[bits 32]
protected_mode_entry_point:
    mov ax, segment_data
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; print via memory mapped VGA
    mov ebx, 0xb8000 + 2 * (14 * 80 + 20)
    mov eax, 0
    mov al, 'C' ; write 'X' in the middle of screen
    mov ah, 0x0f ; white on black
    mov [ds:ebx], eax
    jmp 0x7E00

%include "src/boot/print16.asm"
%include "src/boot/gdt.asm"

times 510 - ($-$$) db 0
dw 0xaa55
