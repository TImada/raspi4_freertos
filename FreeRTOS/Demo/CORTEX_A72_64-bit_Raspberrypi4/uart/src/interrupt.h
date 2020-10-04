#include <stdint.h>

/* Functions */
int isr_register(uint32_t intno, uint32_t pri, uint32_t cpumask, void (*fn)(void));
void eoi_notify(uint32_t val);
void wait_gic_init(void);

/* Interrupt handler table */
typedef void (*INTERRUPT_HANDLER)(void);
typedef struct {
    INTERRUPT_HANDLER fn;
} INTERRUPT_VECTOR;

