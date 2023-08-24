#ifndef HW_MISC_S5L8702_CLK_H
#define HW_MISC_S5L8702_CLK_H

#include "qom/object.h"
#include "hw/sysbus.h"

#define TYPE_S5L8702_CLK "s5l8702-clk"
OBJECT_DECLARE_SIMPLE_TYPE(S5L8702ClkState, S5L8702_CLK)

#define S5L8702_CLK_BASE 0x3C500000
#define S5L8702_CLK_SIZE 0x00100000

#define S5L8702_CLK_NUM_REGS (S5L8702_CLK_SIZE / sizeof(uint32_t))

struct S5L8702ClkState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    uint32_t regs[S5L8702_CLK_NUM_REGS];
};

#endif /* HW_MISC_S5L8702_CLK_H */
