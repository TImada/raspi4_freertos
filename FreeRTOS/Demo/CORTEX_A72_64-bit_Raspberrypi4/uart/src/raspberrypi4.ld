ENTRY(_boot)

STACKTOP = 0x20800000;
 
SECTIONS
{
    /* Starts at 0x20000000 (assuming ends at 0x207FFFFF, 8MB). */
    . = 0x20000000;
    __start = .;
    __text_start = .;
    .text :
    {
        KEEP(*(.vectors))
        KEEP(*(.text.boot))
        *(.text)
    }
    . = ALIGN(4096); /* align to page size */
    __text_end = .;
 
    __data_start = .;
    .data :
    {
        *(.data)
    }
    . = ALIGN(4096); /* align to page size */
    __data_end = .;
 
    __bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)
    }
    . = ALIGN(4096); /* align to page size */
    __bss_end = .;
    __end = .;

    . = STACKTOP ; /* stack memory */
    stack_top = .;
}
__bss_size = (__bss_end - __bss_start)>>3;
