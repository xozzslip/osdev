
[bits 32]
[extern main]
; ; print via memory mapped VGA
; mov ebx, 0xb8000 + 2 * (14 * 80 + 2)
; mov eax, 0
; mov al, 'M' ; write 'X' in the middle of screen
; mov ah, 0x0f ; white on black
; mov [ds:ebx], eax
section .kernel_entry
    call main
    jmp $
