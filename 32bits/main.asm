mov ax, 0xb000
mov ds, ax

mov bx, 0x8000 + 2 * (14 * 80 + 2)
mov al, 'B'
mov ah, WHITE_ON_BLACK
mov [ds:bx], ax

loop:
    jmp loop

WHITE_ON_BLACK equ 0x0f ; the color byte for each character

times 510 - ($-$$) db 0
dw 0xaa55
