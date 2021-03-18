#include <stdint.h>

/* Rasbperry Pi 4B has a 35-bit (up to 32GB) memory region.
 * We consider 2-stage page table walk with the 4KB granularity.
 * 1st stage: 1GB block size
 * 2nd stage: 2MB block size
 */
#define NUM_L1_DESC     (32ULL)
#define NUM_L2_ENTRY    (512ULL)

/* Page tabe size macros */
#define SIZE_1M         (0x100000ULL)
#define SIZE_2M         (0x200000ULL)
#define SIZE_4M         (0x400000ULL)
#define SIZE_8M         (0x800000ULL)
#define SIZE_16M        (0x1000000ULL)
#define SIZE_32M        (0x2000000ULL)
#define SIZE_64M        (0x4000000ULL)
#define SIZE_128M       (0x8000000ULL)
#define SIZE_256M       (0x10000000ULL)
#define SIZE_512M       (0x20000000ULL)
#define SIZE_1G         (0x40000000ULL)
#define SIZE_2G         (0x80000000ULL)
#define SIZE_4G         (0x100000000ULL)

/* Cache policy macros (MAIR_EL1) */
#define TYPE_DEVICE         (0U) /* Meaning Device-nGnRnE */
#define TYPE_MEM_NONCACHE   (1U) /* Inner-Outer non-cacheable */
#define TYPE_MEM_CACHE_WT   (2U) /* Inner-Outer cacheable, write-through */
#define TYPE_MEM_CACHE_WB   (3U) /* Inner-Outer cacheable, write-back */

/* XN-bit control (UXN + PXN bits for level 2) */
#define XN_ON       (0x0ULL)
#define XN_OFF      (0x2ULL)

/* Memory sharable macros (SH) */
#define NON_SHARABLE        (0U)
#define OUTER_SHARABLE      (2U)
#define INNER_SHARABLE      (3U)

/* Memory permission macros (AP[2:1]) */
#define READ_WRITE  (0x0ULL)
#define READ_ONLY   (0x2ULL)

/* Bit mask for 32GB memory region with 4KB granule size */
#define GLANULE_4KB_MASK    (0x3FFFFF000ULL)

/* Page table configuration data definition */
struct ptc_t
{
    uint64_t addr;
    uint64_t size;
    uint64_t executable;
    uint16_t sharable;
    uint8_t permission;
    uint8_t policy;
};
