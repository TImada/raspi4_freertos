/* mmu_cfg.c */
#include "mmu.h"
#include "mmu_cfg.h"

/* Page table configuration array */
struct ptc_t pt_config[NUM_PT_CONFIGS] =
{
    { /* Code region (Read only) */
        .addr = 0x20000000ULL,
        .size = SIZE_2M,
        .executable = XN_OFF,
        .sharable = NON_SHARABLE,
        .permission = READ_WRITE,
        .policy = TYPE_MEM_CACHE_WB,
    },
    { /* Data region */
        .addr = 0x20200000ULL,
        .size = SIZE_4M,
        .executable = XN_ON,
        .sharable = NON_SHARABLE,
        .permission = READ_WRITE,
        .policy = TYPE_MEM_CACHE_WB,
    },
    { /* Page table (Private) */
        .addr = 0x20600000ULL,
        .size = SIZE_2M,
        .executable = XN_ON,
        .sharable = NON_SHARABLE,
        .permission = READ_WRITE,
        .policy = TYPE_MEM_CACHE_WB,
    },
    { /* Peripehral devices (MMIO) */
        .addr = 0xFC000000ULL,
        .size = SIZE_64M,
        .executable = XN_ON,
        .sharable = OUTER_SHARABLE,
        .permission = READ_WRITE,
        .policy = TYPE_DEVICE,
    },
};

