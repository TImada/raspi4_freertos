/* mmu.c */
#include <stdint.h>
#include <string.h>
#include "mmu.h"
#include "mmu_cfg.h"

extern void invalidate_dcache_all(void);
extern struct ptc_t pt_config[NUM_PT_CONFIGS];

static volatile uint64_t sctlr_el1 = 0x0ULL;

/* Page table descriptors and entries */
static __attribute__((aligned(4096))) uint64_t l1ptd[NUM_L1_DESC] __attribute__((section(".pt"))) = {0};
static __attribute__((aligned(4096))) uint64_t l2pte[NUM_L1_DESC * NUM_L2_ENTRY] __attribute__((section(".pt"))) = {0};
static __attribute__((aligned(4096))) uint64_t l1ptd_dummy[NUM_L1_DESC] __attribute__((section(".pt"))) = {0};

/* Zeros all the page table descriptors and entries */
void init_pt(void)
{

    /* 1st stage descriptors */
    memset(l1ptd, 0x0ULL, NUM_L1_DESC * sizeof(uint64_t));
    /* 2nd stage entries */
    memset(l2pte, 0x0ULL, NUM_L1_DESC * NUM_L2_ENTRY * sizeof(uint64_t));

    /* 1st upper (dummy) */
    memset(l1ptd_dummy, 0x0ULL, NUM_L1_DESC * sizeof(uint64_t));

    return;
}

/*  Initialize MMU related registers */
void init_regs(void)
{
    volatile uint64_t reg;

    /* Disable MMU */
    reg = 0x0ULL;
    asm volatile ("msr sctlr_el1, %0" : : "r" (reg));
    asm volatile ("isb");

    /* TLB invalidation */
    asm volatile ("tlbi vmalle1");

    /* Cache flush (data and instruction) */
    asm volatile ("ic iallu");
    invalidate_dcache_all();

    asm volatile ("dsb sy");
    asm volatile ("isb");

    /* MAIR_EL1 configuration */
    reg = ((0xFFULL << (TYPE_MEM_CACHE_WB * 8)) |   /* Write-back cache enabled memory */
           (0xBBULL << (TYPE_MEM_CACHE_WT * 8)) |   /* Write-through cache enabled memory */
           (0x44ULL << (TYPE_MEM_NONCACHE * 8)) |   /* Cache disabled memory */
           (0x00ULL << (TYPE_DEVICE * 8)));         /* Device memory */
    asm volatile ("msr mair_el1, %0" : : "r" (reg));
    asm volatile ("isb");

    /* TTBR{0,1}_EL1 configuration */
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((uint64_t)&l1ptd[0]));
    asm volatile ("msr ttbr1_el1, %0" : : "r" ((uint64_t)&l1ptd_dummy[0]));
    asm volatile ("isb");

    /* TCR_EL1 configuration */
    reg = ((0x1ULL << 38) | /* Top byte not ignored */
           (0x1ULL << 37) | /* Top byte not ignored */
           (0x1ULL << 36) | /* 16-bit ASID */
           (0x1ULL << 32) | /* 36-bit IPS */
           (0x0ULL << 30) | /* 4KB granule size for TG1 */
           (0x2ULL << 28) | /* Outer sharable */
           (0x1ULL << 26) | /* Outer write-back cacheable */
           (0x1ULL << 24) | /* Inner write-back cacheable */
           (0x1ULL << 23) | /* TTBR1_EL1 page table walk disabled */
           (29ULL << 16)  | /* 32GB region size */
           (0x0ULL << 14) | /* 4KB granule size for TG0 */
           (0x2ULL << 12) | /* Outer sharable */
           (0x1ULL << 10) | /* Outer write-back cacheable */
           (0x1ULL << 8)  | /* Inner write-back cacheable */
           (29ULL << 0));   /* 32GB region size */
    asm volatile ("msr tcr_el1, %0" : : "r" (reg));
    asm volatile ("isb");

    return;
}

/* Set page tables */
void set_pt(struct ptc_t *config)
{
    volatile uint64_t pi, num, reg;
    uint64_t i, j;
    uint64_t index;

    /* General page table configuration */
    for (i = 0; i < NUM_L1_DESC; i++) {
        /* 1st stage page table (overwrite) */
        l1ptd[i] = (((((uint64_t)&l2pte[i * NUM_L2_ENTRY] & GLANULE_4KB_MASK) >> 12 ) << 12) | /* 2nd stage entry address */
                   (0x0ULL << 63) | /* NStable (ignored for NS) */
                   (0x0ULL << 61) | /* APTable disabled */
                   (0x0ULL << 60) | /* UXNTable disabled */
                   (0x0ULL << 59) | /* PXNTable disabled */
                   (0x3ULL << 0));  /* Entry type */

        /* 2nd stage page table */
        for (j = 0; j < NUM_L2_ENTRY; j++) {
            reg = ((XN_ON << 53)                |  /* UXN + PXN */
                ((i * NUM_L2_ENTRY + j) << 21)  |  /* Output address */
                (0x1ULL << 10)                  |  /* Access flag */
                (OUTER_SHARABLE << 8)           |  /* Sharablility */
                (READ_WRITE << 6)               |  /* Access permission */
                (TYPE_DEVICE << 2)              |  /* Policy index */
                (0x1ULL << 0));                    /* Entry type */
            l2pte[i * NUM_L2_ENTRY + j] = reg;
        }
    }

    /* User defined page table configuration */
    for (i = 0; i < NUM_PT_CONFIGS; i++) {
        /* 1st stage page table (overwrite) */
        index = (config[i].addr >> 30);
        l1ptd[index] = (((((uint64_t)&l2pte[index * NUM_L2_ENTRY] & GLANULE_4KB_MASK) >> 12 ) << 12) | /* 2nd stage entry address */
                   (0x0ULL << 63) | /* NStable (ignored for NS) */
                   (0x0ULL << 61) | /* APTable disabled */
                   (0x0ULL << 60) | /* UXNTable disabled */
                   (0x0ULL << 59) | /* PXNTable disabled */
                   (0x3ULL << 0));  /* Entry type */

        pi = config[i].addr >> 21;  /* Page table entry index */
        num = config[i].size >> 21;

        /* 2nd stage page table */
        for (j = 0; j < num; j++) {
            reg = ((config[i].executable << 53) |  /* UXN + PXN */
                ((pi + j) << 21)                |  /* Output address */
                (0x1ULL << 10)                  |  /* Access flag */
                (config[i].sharable << 8)       |  /* Sharablility */
                (config[i].permission << 6)     |  /* Access permission */
                (config[i].policy << 2)         |  /* Policy index */
                (0x1ULL << 0));                    /* Entry type */
            l2pte[pi + j] = reg;
        }
    }

    return;
}

/* Update page tables */
void update_pt(void)
{
    volatile uint64_t reg;

    /* Set page pables */
    set_pt(&pt_config[0]);

    /* TTBR{0,1}_EL1 configuration */
    asm volatile ("msr ttbr0_el1, %0" : : "r" (0x0ULL));
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((uint64_t)&l1ptd[0]));
    asm volatile ("isb");

    /* Enable MMU */
    asm volatile ("mrs %0, sctlr_el1" : "=r" (reg));
    reg |= ((0x1ULL << 12) |    /* I-cache enabled */
            (0x1ULL << 4)  |    /* EL0 stack alignment check enabled */
            (0x1ULL << 3)  |    /* Stack alignment check enabled */
            (0x1ULL << 2)  |    /* D-cache enabled */
            (0x0ULL << 1)  |    /* Alignment check disabled */
            (0x1ULL << 0));     /* MMU enabled */
    reg &= ~(0x1ULL << 19);
    asm volatile ("msr sctlr_el1, %0" : : "r" (reg));
    asm volatile ("dsb sy");
    asm volatile ("isb");

    return;
}

/* Main MMU configuration */
void configure_mmu(void)
{
    init_regs();
    update_pt();

    return;
}
/*-----------------------------------------------------------*/

