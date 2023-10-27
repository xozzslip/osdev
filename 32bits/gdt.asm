; https://wiki.osdev.org/Global_Descriptor_Table

gdt_start:
gdt_null:
    dd 0
    dd 0
gdt_code:
    dw 0xffff ; limit  0-15
    dw 0x0000 ; base  16-31
    db 0x00   ; base  32-39
    db 0x9a   ; access byte: executable hex(int("10011010", 2))
    db 0xfc   ; limit (4 bits) + flags (4 bits)
    db 0x00   ; base  56-63
gdt_data:
    dw 0xffff ; limit  0-15
    dw 0x0000 ; base  16-31
    db 0x00   ; base  32-39
    db 0x92   ; access byte: not exec
    db 0xfc   ; limit (4 bits) + flags (4 bits)
    db 0x00   ; base  56-63
gdt_end:


gdt_descriptor_start:
    dw gdt_end - gdt_start - 1 ; this points not to size, but to last valid offset actually. This is why here we have -1
    dd gdt_start
