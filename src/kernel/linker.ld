MEMORY
{
    /* Right now we load only with 63.5K from disk from BIOS  */
    /* OS loaded right after boot sector at 0x7E00 */
    RAM : ORIGIN = 0x7E00, LENGTH = 63K
}


SECTIONS
{
    .text : {
        *(.kernel_entry) /* Binary MUST start from entry.asm */
        *(.text)
    } >RAM

    .data : {
        *(.data)
    } >RAM

    /* right now bss take up space which is good since we don't have runtime that would initialize this sector anyway
    (though my understanding of that is limited) */
    .bss : {
        *(.bss)
    } > RAM
}
