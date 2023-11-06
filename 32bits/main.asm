[org 0x7c00]
    mov bp, 0x8000
    mov sp, bp
    mov bx, real_mode_message
    call print_str
    call print_nl

    cli
    lgdt [gdt_descriptor_start]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp segment_code:print_something



print_something:
    mov ax, segment_data
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
[bits 32]
    ; mov ebx, 0xb8000 + 2 * (14 * 80 + 2)
    mov ebx, 0x000b88c4

    mov eax, 0
    mov al, 'C'
    mov ah, WHITE_ON_BLACK

    mov [ds:ebx], eax

[bits 16]
loop:
    jmp loop



%include "gdt.asm"
%include "../16bits/print.asm"
WHITE_ON_BLACK equ 0x0f ; the color byte for each character

real_mode_message:
    db "Running in real mode.", 0

times 510 - ($-$$) db 0
dw 0xaa55
