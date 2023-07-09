#include "interrupt.h"
#include "board.h"

/* Vector table */
INTERRUPT_VECTOR InterruptHandlerFunctionTable[MAX_NUM_IRQS] = {0};

/* Interrupt handler registration */
int isr_register(uint32_t intno, uint32_t pri, uint32_t cpumask, void (*fn)(void))
{
    uint32_t n, reg, shift;
    uint32_t *addr;

    if (intno > 0xFFU) {
        return -1;
    }
    if (pri > 0xFFU) {
        return -2;
    }
    if (cpumask > 0xFFU) {
        return -3;
    }
    if (!fn) {
        return -4;
    }

    /* GICD_ISENABLERn */
    n = intno / 32U;
    addr = (uint32_t *)(GICD_BASE + 0x100U + 4U * n);
    reg = *addr;
    *addr = (reg | (0x1U << (intno % (32U))));
    
    /* GICD_IPRIORITYRn */
    n = intno / 4U;
    addr = (uint32_t *)(GICD_BASE + 0x400U + 4U * n);
    shift = (intno % 4U) * 8U;
    reg = (*addr) & ~(0xFFU << shift);
    *addr = (reg | pri << shift);
    
    /* GICD_ITARGETSRn (only for SPIs) */
    if (intno >= 32U) {
        n = intno / 4U;
        addr = (uint32_t *)(GICD_BASE + 0x800U + 4U * n);
        shift = (intno % 4U) * 8U;
        reg = (*addr) & ~(0xFFU << shift);
        *addr = (reg | cpumask << shift);
    }
    asm volatile ("isb");

    /* Handler registration */
    InterruptHandlerFunctionTable[intno].fn = fn;
    
    return 0;
}   
/*-----------------------------------------------------------*/

/* EOI notification */
void eoi_notify(uint32_t val)
{
    uint32_t *addr;

    addr = (uint32_t *)(GICC_BASE + 0x10U);
    *addr = val;
    asm volatile ("isb");

    return;
}   
/*-----------------------------------------------------------*/

/*
 * wait_gic_init()
 * To check GIC initialization by Linux
 */
void wait_gic_init(void)
{
    volatile uint32_t *addr;

    addr = (uint32_t *)(GICD_BASE + 0x00U);

    while (*addr == 0x1U) { /* Wait until Linux disables GICD to set it up */
        ;
    }
    while (*addr == 0x0U) { /* Wait until Linux enables GICD again after completing GICD setting up */
        ;
    }

    return;
}
/*-----------------------------------------------------------*/
