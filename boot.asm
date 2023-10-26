[org 0x7c00]
; db 'X'
mystring:
    db 'ABCCCCCCA', 0

mov ah, 0x0e
mov bp, 0x8000
mov sp, bp

mov bx, mystring
call print_str
call print_nl
call print_str
call print_nl



; mov ax, [mystring]
; call print_char


loop:
    jmp loop

%include "print.asm"

times 510 - ($-$$) db 0
dw 0xaa55
