#ifndef HW_TIMER_S5L8702_TIMER_H
#define HW_TIMER_S5L8702_TIMER_H

#include "qom/object.h"
#include "hw/sysbus.h"

#define TYPE_S5L8702_TIMER  "s5l8702-timer"
OBJECT_DECLARE_SIMPLE_TYPE(S5L8702TimerState, S5L8702_TIMER)

#define S5L8702_TIMER_BASE  0x3C700000
#define S5L8702_TIMER_SIZE  0x00100000

#define S5L8702_TIMER_NUM_REGS  (S5L8702_TIMER_SIZE / sizeof(uint32_t))

struct S5L8702TimerState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    uint32_t regs[S5L8702_TIMER_NUM_REGS];
};

#endif /* HW_TIMER_S5L8702_TIMER_H */
