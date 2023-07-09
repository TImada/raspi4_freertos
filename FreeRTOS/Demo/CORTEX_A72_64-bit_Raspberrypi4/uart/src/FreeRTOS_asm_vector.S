.org 0
.text

    .globl _freertos_vector_table

.section .vectors
.balign 2048
_vector_table:

.set VBAR, _vector_table

.org VBAR
    b    _boot
.org (VBAR + 0x80)
    b    .
.org (VBAR + 0x100)
    b    .
.org (VBAR + 0x180)
    b    .

.org (VBAR + 0x200)
    b    .
.org (VBAR + 0x280)
    b    .
.org (VBAR + 0x300)
    b    .
.org (VBAR + 0x380)
    b    .

.org (VBAR + 0x400)
    b    .
.org (VBAR + 0x480)
    b    .
.org (VBAR + 0x500)
    b    .
.org (VBAR + 0x580)
    b    .

.org (VBAR + 0x600)
    b    .
.org (VBAR + 0x680)
    b    .
.org (VBAR + 0x700)
    b    .
.org (VBAR + 0x780)
    b    .

/******************************************************************************
 * Vector table to use when FreeRTOS is running.
 *****************************************************************************/
.set    FREERTOS_VBAR, (VBAR+0x1000)

.org(FREERTOS_VBAR)
_freertos_vector_table:
    b    FreeRTOS_SWI_Handler
.org (FREERTOS_VBAR + 0x80)
    b    FreeRTOS_IRQ_Handler
.org (FREERTOS_VBAR + 0x100)
    b    .
.org (FREERTOS_VBAR + 0x180)
    b    .

.org (FREERTOS_VBAR + 0x200)
    b    FreeRTOS_SWI_Handler
.org (FREERTOS_VBAR + 0x280)
    b    FreeRTOS_IRQ_Handler
.org (FREERTOS_VBAR + 0x300)
    b    .
.org (FREERTOS_VBAR + 0x380)
    b    .

.org (FREERTOS_VBAR + 0x400)
    b    .
.org (FREERTOS_VBAR + 0x480)
    b    .
.org (FREERTOS_VBAR + 0x500)
    b    .
.org (FREERTOS_VBAR + 0x580)
    b    .

.org (FREERTOS_VBAR + 0x600)
    b    .
.org (FREERTOS_VBAR + 0x680)
    b    .
.org (FREERTOS_VBAR + 0x700)
    b    .
.org (FREERTOS_VBAR + 0x780)
    b    .

.end
