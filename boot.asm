[org 0x7c00]
mov bp, 0x8000
mov sp, bp
mov bx, mystring
call print_str
call print_nl
call print_str
call print_nl

mov bx, 0x0004
call print_register
call print_nl

; reading from disk
mov bx, 0x9000 ; memory address where data will be loaded to
mov ah, 2 ; int 0x13 command
mov al, 2 ; number of sectors to read (read two sectors)
mov cl, 2 ; sector number  (1-17 dec.) enumerated from 1!
mov ch, 0 ; track number is zero, hopefully that track has a few sectors
; dl should be already set by BIOS
mov dh, 0
int 0x13

cmp ah, 0
je loop

mov bx, disk_error_string
call print_str 




loop:
    jmp loop

; data section
mystring:
    db 'ABCCCCCCA', 0

disk_error_string:
    db 'DISK ERROR', 0

%include "print.asm"


times 510 - ($-$$) db 0
dw 0xaa55

times 510 db 0
dw 0xdada

times 510 db 0
dw 0xface
