.section ".text.boot"
 
// Make _start global.
.globl _boot
 
_boot:
    // This program focuses on only core #3.
    // The other cores will return.
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cmp     x1, #3
    bne     kick_core3

start_el2:
    // enable AArch64 in EL1.
    mov     x0, #(1 << 31)      // AArch64
    orr     x0, x0, #(1 << 1)   // SWIO hardwired on Pi4
    msr     hcr_el2, x0
    isb
    // change execution level to EL1.
    mov     x2, #0x3c5         // D=1, A=1, I=1, F=1 M=EL1h
    msr     spsr_el2, x2
    isb
    adr     x2, start_el1
    msr     elr_el2, x2
    isb
    eret

start_el1:
    // enable FPU
    mrs     x1, cpacr_el1
    mov     x0, #(3 << 20)
    orr     x0, x1, x0
    msr     cpacr_el1, x0
    // set sp
    adrp    x1, stack_top
    mov     sp, x1
    // configure MMU
    ldr     x0, =configure_mmu
    blr     x0
    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
1:  cbz     w2, 2f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 1b

2:  bl      main
    ret

kick_core3:
    mov     x1, #0xF0 // See https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S
    ldr     x2, =__start
    str     x2, [x1]
    sev
    wfe
    mov     x0, #0x0
    ret
