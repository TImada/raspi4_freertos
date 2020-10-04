/* GIC-400 registers */
#define GIC_BASE	(0xFF840000UL)
#define GICD_BASE	(GIC_BASE + 0x00001000UL)
#define GICC_BASE	(GIC_BASE + 0x00002000UL)

/* The number of IRQs on BCM2711 */
#define MAX_NUM_IRQS (224U)

/* IRQ number */
#define IRQ_VTIMER (27)
#define IRQ_VC_UART (153)

