extern isr_handler

%macro ISR_NOERRCODE 1  ; define a macro, taking one parameter
  global isr%1        ; %1 accesses the first parameter.
  isr%1:
    cli
    push 0x00 ; this is dummy error code
    push byte %1
    pusha ; EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    call isr_handler
    popa
    add esp, 0x08 ; this pops two entries
    sti
    ; interrupt pushes 3 4-byte values onto the stack
    ; they are EIP, and two more (code segment I guess and some flags)
    ; iret pops up those values
    iret
%endmacro

%macro ISR_WITH_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1 ; error code was pushed implicitly
    pusha
    call isr_handler
    popa
    add esp, 0x08 ; this pops two entries (one of them is an error)
    sti
    iret
%endmacro

ISR_NOERRCODE    0  ; 0: Divide By Zero Exception
ISR_NOERRCODE    1  ; 1: Debug Exception
ISR_NOERRCODE    2  ; 2: Non Maskable Interrupt Exception
ISR_NOERRCODE    3  ; 3: Int 3 Exception
ISR_NOERRCODE    4  ; 4: INTO Exception
ISR_NOERRCODE    5  ; 5: Out of Bounds Exception
ISR_NOERRCODE    6  ; 6: Invalid Opcode Exception
ISR_NOERRCODE    7  ; 7: Coprocessor Not Available Exception
ISR_WITH_ERRCODE 8  ; 8: Double Fault Exception (With Error Code!)
ISR_NOERRCODE    9  ; 9: Coprocessor Segment Overrun Exception
ISR_WITH_ERRCODE 10 ; 10: Bad TSS Exception (With Error Code!)
ISR_WITH_ERRCODE 11 ; 11: Segment Not Present Exception (With Error Code!)
ISR_WITH_ERRCODE 12 ; 12: Stack Fault Exception (With Error Code!)
ISR_WITH_ERRCODE 13 ; 13: General Protection Fault Exception (With Error Code!)
ISR_WITH_ERRCODE 14 ; 14: Page Fault Exception (With Error Code!)
ISR_NOERRCODE    15 ; 15: Reserved Exception
ISR_NOERRCODE    16 ; 16: Floating Point Exception
ISR_NOERRCODE    17 ; 17: Alignment Check Exception
ISR_NOERRCODE    18 ; 18: Machine Check Exception
ISR_NOERRCODE    19 ; 19: Reserved
ISR_NOERRCODE    20 ; 20: Reserved
ISR_NOERRCODE    21 ; 21: Reserved
ISR_NOERRCODE    22 ; 22: Reserved
ISR_NOERRCODE    23 ; 23: Reserved
ISR_NOERRCODE    24 ; 24: Reserved
ISR_NOERRCODE    25 ; 25: Reserved
ISR_NOERRCODE    26 ; 26: Reserved
ISR_NOERRCODE    27 ; 27: Reserved
ISR_NOERRCODE    28 ; 28: Reserved
ISR_NOERRCODE    29 ; 29: Reserved
ISR_NOERRCODE    30 ; 30: Reserved
ISR_NOERRCODE    31 ; 31: Reserved
