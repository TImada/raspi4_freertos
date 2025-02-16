ENTRY(_boot)

CODE_BASE   = 0x20000000;
CODE_SIZE   = 0x00200000;
DATA_BASE   = CODE_BASE + CODE_SIZE;
DATA_SIZE   = 0x00400000;
STACK_TOP   = DATA_BASE + DATA_SIZE; /* STACK in the upper end of DATA */
PT_BASE     = DATA_BASE + DATA_SIZE;
PT_SIZE     = 0x00200000;

MEMORY {
    CODE  (rx) : ORIGIN = CODE_BASE, LENGTH = CODE_SIZE
    DATA  (rw) : ORIGIN = DATA_BASE, LENGTH = DATA_SIZE
    PT    (rw) : ORIGIN = PT_BASE,   LENGTH = PT_SIZE
}
 
SECTIONS
{
    /* Starts at 0x20000000 (assuming ends at 0x208FFFFF, 9MB). */
    .text :
    {
        __start = .;
        __text_start = .;
        KEEP(*(.vectors))
        KEEP(*(.text.boot))
        *(.text)
        __text_end = .;
    } > CODE
 
    .data :
    {
        . = ALIGN(4096); /* align to page size */
        __data_start = DATA_BASE;
        *(.data)
        __data_end = .;
    } > DATA
 
    .bss :
    {
        . = ALIGN(4096); /* align to page size */
        __bss_start__ = .;
        __bss_start = .;
        bss = .;
        *(.bss)
        __bss_end__ = .;
        __bss_end = .;
    } > DATA

    .heap :
    {
        . = ALIGN(4096); /* align to page size */
        __heap_start__ = .;
        end = __heap_start__;
        _end = end;
        __end = end;
        __end__ = end;
        KEEP(*(.heap))
        __heap_end__ = .;
        __HeapLimit = __heap_end__;
    } > DATA

    .stack :
    {
    } > DATA
    stack_top = STACK_TOP;

    .pt :
    {
        . = ALIGN(4096); /* align to page size */
        *(.pt)
    } > PT

}
__bss_size = (__bss_end - __bss_start)>>3;
