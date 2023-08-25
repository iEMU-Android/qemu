#ifndef HW_MISC_S5L8702_SHA_H
#define HW_MISC_S5L8702_SHA_H

#include "qom/object.h"
#include "hw/sysbus.h"

#define TYPE_S5L8702_SHA    "s5l8702-sha"
OBJECT_DECLARE_SIMPLE_TYPE(S5L8702ShaState, S5L8702_SHA)

#define S5L8702_SHA_BASE    0x38000000
#define S5L8702_SHA_SIZE    0x00100000

#define S5L8702_SHA_NUM_REGS    (S5L8702_SHA_SIZE / sizeof(uint32_t))

struct S5L8702ShaState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    uint32_t regs[S5L8702_SHA_NUM_REGS];
};

#endif /* HW_MISC_S5L8702_SHA_H */
