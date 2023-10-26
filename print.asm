; print string, pass string by "mov bx, mystring"
print_str:
    pusha    
    mov ah, 0x0e
print_next_char:
    mov cx, [bx]
    cmp cl, 0
    je print_str_end
    mov al, cl    
    int 0x10
    add bx, 1
    jmp print_next_char
print_str_end:
    popa
    ret

; print hex value of bx register
print_register:
    pusha
    mov cx, bx
    and cx, 0x000f
    cmp cx, 9
    jle print_numerical_char
    ; print A-F
    sub cx, 9
    add cx, 0x41
    mov al, cl
    call print_char
    jmp print_register_end
print_numerical_char:
    ; print 0-9
    add cx, 0x30
    mov al, cl
    call print_char
    jmp print_register_end
print_register_end:
    popa
    ret

; reserve memory for register content printing
register_string:
    db "00 00"


; print new line
print_nl:
    pusha
    mov al, 0x0A
    call print_char
    mov al, 0x0D
    call print_char
    popa
    ret

; print character
print_char:
    pusha
    mov ah, 0x0e
    int 0x10
    popa
    ret
