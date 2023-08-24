#ifndef HW_MISC_S5L8702_AES_H
#define HW_MISC_S5L8702_AES_H

#include "qom/object.h"
#include "hw/sysbus.h"

#define TYPE_S5L8702_AES "s5l8702-aes"
OBJECT_DECLARE_SIMPLE_TYPE(S5L8702AesState, S5L8702_AES)

#define S5L8702_AES_BASE 0x38C00000
#define S5L8702_AES_SIZE 0x00100000

#define S5L8702_AES_NUM_REGS (S5L8702_AES_SIZE / sizeof(uint32_t))

struct S5L8702AesState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    uint32_t regs[S5L8702_AES_NUM_REGS];
};

#endif /* HW_MISC_S5L8702_AES_H */
