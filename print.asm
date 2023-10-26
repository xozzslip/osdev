; print string
print_str:
    pusha    
    mov ah, 0x0e
print_next_char:
    mov cx, [bx]
    cmp cl, 0
    je print_return
    mov al, cl    
    int 0x10
    add bx, 1
    jmp print_next_char
print_return:
    popa
    ret

; print hex
print_str:
    pusha    
    mov bx, ax
    mov ah, 0x0e
print_next_char:
    mov cx, [bx]
    cmp cl, 0
    je print_return
    mov al, cl    
    int 0x10
    add bx, 1
    jmp print_next_char
print_return:
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
