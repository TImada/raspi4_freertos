/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "interrupt.h"
#include "board.h"
#include "uart.h"

/* Vector table */
extern INTERRUPT_VECTOR InterruptHandlerFunctionTable[MAX_NUM_IRQS];

/* ARM Generic Timer */
static uint32_t timer_cntfrq = 0;
static uint32_t timer_tick = 0;

void enable_cntv(void)
{
    uint32_t cntv_ctl;
    cntv_ctl = 1;
    asm volatile ("msr cntv_ctl_el0, %0" :: "r" (cntv_ctl));
}
/*-----------------------------------------------------------*/

void write_cntv_tval(uint32_t val)
{
    asm volatile ("msr cntv_tval_el0, %0" :: "r" (val));
    return;
}
/*-----------------------------------------------------------*/

uint32_t read_cntfrq(void)
{
    uint32_t val;
    asm volatile ("mrs %0, cntfrq_el0" : "=r" (val));
    return val;
}
/*-----------------------------------------------------------*/

void init_timer(void)
{
    timer_cntfrq = timer_tick = read_cntfrq();
    write_cntv_tval(timer_cntfrq);    // clear cntv interrupt and set next 1 sec timer.
    return;
}
/*-----------------------------------------------------------*/

void timer_set_tick_rate_hz(uint32_t rate)
{
    timer_tick = timer_cntfrq / rate ;
    write_cntv_tval(timer_tick);
}
/*-----------------------------------------------------------*/

void vConfigureTickInterrupt( void )
{
    /* init timer device. */
    init_timer();

    /* set tick rate. */
    timer_set_tick_rate_hz(configTICK_RATE_HZ);

    /* start the timer. */
    enable_cntv();

    /* register the time isr */
    isr_register(IRQ_VTIMER, 0xA0U, (0x1U << 0x3U), FreeRTOS_Tick_Handler);

    return;
}
/*-----------------------------------------------------------*/

void vClearTickInterrupt( void )
{
    write_cntv_tval(timer_tick);    // clear cntv interrupt and set next timer.
    return;
}
/*-----------------------------------------------------------*/

void vApplicationIRQHandler( uint32_t ulICCIAR )
{
    uint32_t ulInterruptID, val;

    /* The ID of the interrupt can be obtained by bitwise ANDing the ICCIAR value
    with 0x3FF. */
    ulInterruptID = ulICCIAR & (0x3FFU);

    /* EOI notification */
    if (ulInterruptID < 0x10) {
        val = ((0x3U << 10) | ulInterruptID);
#if 0
    } else if (ulInterruptID == 0x3ff) {
        return;
#endif
    } else {
        val = ulInterruptID;
    }
    eoi_notify(val);

    if (val >= MAX_NUM_IRQS) {
        return;
    }

    /* On the assumption that handlers for each interrupt are stored in an array
    called InterruptHandlerFunctionTable, use the interrupt ID to index to and
    call the relevant handler function. */
    if (!(InterruptHandlerFunctionTable[ulInterruptID].fn)) {
        return;
    }
    InterruptHandlerFunctionTable[ulInterruptID].fn();

    return;
}

