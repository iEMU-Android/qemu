#ifndef HW_INTC_PL192
#define HW_INTC_PL192

#include "hw/irq.h"
#include "hw/sysbus.h"
#include "qom/object.h"

/* The number of virtual priority levels. 32 user vectors plus the
   unvectored IRQ. Chained interrupts would require an additional level
   if implemented. */
#define PL192_NUM_PRIO 33

#define TYPE_PL192 "pl192"
OBJECT_DECLARE_SIMPLE_TYPE(PL192State, PL192)

struct PL192State {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    uint32_t level;
    uint32_t soft_level;
    uint32_t irq_enable;
    uint32_t fiq_select;
    uint8_t vect_control[16];
    uint32_t vect_addr[PL192_NUM_PRIO];
    /* Mask containing interrupts with higher priority than this one. */
    uint32_t prio_mask[PL192_NUM_PRIO + 1];
    int protected;
    /* Current priority level.  */
    int priority;
    int prev_prio[PL192_NUM_PRIO];
    qemu_irq irq;
    qemu_irq fiq;
};

#endif /* HW_INTC_PL192 */