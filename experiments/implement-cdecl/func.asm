section .text
    global bar
    extern mur

    bar:
        ;  save stack frame
        push ebp
        mov ebp, esp

        ; local variables
        sub esp, 0x10
        mov [ebp-0x0c], DWORD 5
        mov [ebp-0x10], DWORD 6
        mov [ebp-0x08], DWORD 0

        ; call mur
        mov eax, [ebp-0x0c]
        push eax
        mov eax, [ebp+0x8]
        push eax
        call mur
        pop edx
        pop edx

        mov [ebp-0x08], eax

        ; call mur
        mov eax, [ebp-0x10]
        push eax
        mov eax, [ebp+0xc]
        push eax
        call mur
        pop edx
        pop edx

        add eax, [ebp-0x08]

        mov esp, ebp
        pop ebp
        ret








