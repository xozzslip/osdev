; print string, pass string by "mov bx, mystring"
print16_str:
    pusha
    mov ah, 0x0e
_print16_next_char:
    mov cx, [bx]
    cmp cl, 0
    je _print16_str_end
    mov al, cl
    int 0x10
    add bx, 1
    jmp _print16_next_char
_print16_str_end:
    popa
    ret

; _print16 hex value of bx register
print16_register:
    pusha
    mov al, '0'
    call print16_char
    mov al, 'x'
    call print16_char
    mov dx, 0
_print16_register_step:
    mov cx, bx
    and cx, 0xf000
    shr cx, 12
    cmp cx, 9
    jle _print16_numerical_char
    ; _print16 A-F
    sub cx, 10
    add cx, 0x41
    mov al, cl
    call print16_char
    jmp _print16_numerical_char_end
_print16_numerical_char:
    ; _print16 0-9
    add cx, 0x30
    mov al, cl
    call print16_char
    jmp _print16_numerical_char_end
_print16_numerical_char_end:
    shl bx, 4
    add dx, 1
    cmp dx, 4
    jne _print16_register_step
    popa
    ret

; _print16 new line
print16_nl:
    pusha
    mov al, 0x0A
    call print16_char
    mov al, 0x0D
    call print16_char
    popa
    ret

; mov al, 'X'
; _print16 character from al register
print16_char:
    pusha
    mov ah, 0x0e
    int 0x10
    popa
    ret
