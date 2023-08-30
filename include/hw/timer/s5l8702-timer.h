#ifndef HW_TIMER_S5L8702_TIMER_H
#define HW_TIMER_S5L8702_TIMER_H

#include "qom/object.h"
#include "hw/sysbus.h"
#include "qemu/timer.h"
#include "hw/clock.h"
#include "hw/irq.h"

#define TYPE_S5L8702_TIMER  "s5l8702-timer"
OBJECT_DECLARE_SIMPLE_TYPE(S5L8702TimerCtrlState, S5L8702_TIMER)

#define S5L8702_TIMER_BASE  0x3C700000
#define S5L8702_TIMER_SIZE  0x00100000

#define S5L8702_TIMER_COUNT_16  4
#define S5L8702_TIMER_COUNT_32  4
#define S5L8702_TIMER_COUNT     (S5L8702_TIMER_COUNT_16 + S5L8702_TIMER_COUNT_32)

typedef struct S5L8702Timer {
    S5L8702TimerCtrlState *ctrl;
    Clock pclk;
    Clock extclk;
    QEMUTimer timer;
    qemu_irq irq;
    
    uint32_t tcon;
    uint32_t tcmd;
    uint32_t tdata0;
    uint32_t tdata1;
    uint32_t tpre;
    uint32_t tcnt;
} S5L8702Timer;

struct S5L8702TimerCtrlState {
    /*< private >*/
    SysBusDevice parent_obj;

    /*< public >*/
    MemoryRegion iomem;
    Clock *pclk;
    Clock *extclk;
    S5L8702Timer timer[S5L8702_TIMER_COUNT];
    uint32_t tstat;
};

#endif /* HW_TIMER_S5L8702_TIMER_H */
