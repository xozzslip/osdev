; [org 0x7c00]
; db 'X'

mov ah, 0x0e

mov bp, 0x8000
mov sp, bp

push 'Z'
mov ah, 'Y'
mov al, 'X'
call print
mov bh, ah
mov al, bh
call print
pop bx
mov al, bl
call print



loop:
    jmp loop

%include "print.asm"

times 510 - ($-$$) db 0
dw 0xaa55
