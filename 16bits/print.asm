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
    mov al, '0'
    call print_char
    mov al, 'x'
    call print_char
    mov dx, 0    
print_register_step:    
    mov cx, bx
    and cx, 0xf000
    shr cx, 12
    cmp cx, 9
    jle print_numerical_char
    ; print A-F
    sub cx, 10
    add cx, 0x41
    mov al, cl
    call print_char
    jmp print_numerical_char_end
print_numerical_char:
    ; print 0-9
    add cx, 0x30
    mov al, cl
    call print_char
    jmp print_numerical_char_end
print_numerical_char_end:
    shl bx, 4
    add dx, 1
    cmp dx, 4    
    jne print_register_step
    popa
    ret

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
