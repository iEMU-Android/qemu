#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/timer/s5l8702-timer.h"

#define REG_INDEX(offset) (offset / sizeof(uint32_t))

static uint64_t s5l8702_timer_read(void *opaque, hwaddr offset,
                                      unsigned size)
{
    const S5L8702TimerState *s = S5L8702_TIMER(opaque);
    const uint32_t idx = REG_INDEX(offset);

    switch (offset) {
    case 0x0018:
    case 0x001C:
    case 0x002C:
    case 0x003C:
    case 0x005C:
    case 0x0060:
    case 0x0064:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented read offset 0x%04x\n",
                      __func__, (uint32_t) offset);
    }

    return s->regs[idx];
}

static void s5l8702_timer_write(void *opaque, hwaddr offset,
                                   uint64_t val, unsigned size)
{
    S5L8702TimerState *s = S5L8702_TIMER(opaque);
    const uint32_t idx = REG_INDEX(offset);

    switch (offset) {
    case 0x0018:
    case 0x001C:
    case 0x002C:
    case 0x003C:
    case 0x005C:
    case 0x0060:
    case 0x0064:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented write offset 0x%04x\n",
                      __func__, (uint32_t) offset);
    }

    s->regs[idx] = (uint32_t) val;
}

static const MemoryRegionOps s5l8702_timer_ops = {
    .read = s5l8702_timer_read,
    .write = s5l8702_timer_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    // .valid = {
    //     .min_access_size = 4,
    //     .max_access_size = 4,
    // },
    // .impl.min_access_size = 4,
};

static void s5l8702_timer_reset(DeviceState *dev)
{
    S5L8702TimerState *s = S5L8702_TIMER(dev);

    printf("s5l8702_timer_reset\n");

    /* Reset registers */
    memset(s->regs, 0, sizeof(s->regs));

    /* Set default values for registers */
    // s->regs[REG_INDEX(REG_TIMERCON0)] = REG_TIMERCON0_RST;
    // s->regs[REG_INDEX(REG_TIMERCON1)] = REG_TIMERCON1_RST;
    // s->regs[REG_INDEX(REG_TIMERCON2)] = REG_TIMERCON2_RST;
    // s->regs[REG_INDEX(REG_TIMERCON3)] = REG_TIMERCON3_RST;
    // s->regs[REG_INDEX(REG_TIMERCON4)] = REG_TIMERCON4_RST;
    // s->regs[REG_INDEX(REG_TIMERCON5)] = REG_TIMERCON5_RST;
    // // s->regs[REG_INDEX(REG_UNK1)] = REG_UNK1_RST;
    // // s->regs[REG_INDEX(REG_UNK2)] = REG_UNK2_RST;
    // s->regs[REG_INDEX(REG_PLL0PMS)] = REG_PLL0PMS_RST;
    // s->regs[REG_INDEX(REG_PLL1PMS)] = REG_PLL1PMS_RST;
    // s->regs[REG_INDEX(REG_PLL2PMS)] = REG_PLL2PMS_RST;
    // // s->regs[REG_INDEX(REG_UNK3)] = REG_UNK3_RST;
    // s->regs[REG_INDEX(REG_PLL0LCNT)] = REG_PLL0LCNT_RST;
    // s->regs[REG_INDEX(REG_PLL1LCNT)] = REG_PLL1LCNT_RST;
    // s->regs[REG_INDEX(REG_PLL2LCNT)] = REG_PLL2LCNT_RST;
    // // s->regs[REG_INDEX(REG_UNK4)] = REG_UNK4_RST;
    // s->regs[REG_INDEX(REG_PLLLOCK)] = REG_PLLLOCK_RST;
    // s->regs[REG_INDEX(REG_PLLMODE)] = REG_PLLMODE_RST;
    // s->regs[REG_INDEX(REG_PWRCON0)] = REG_PWRCON0_RST;
    // s->regs[REG_INDEX(REG_PWRCON1)] = REG_PWRCON1_RST;
    // s->regs[REG_INDEX(REG_SWRCON)] = REG_SWRCON_RST;
    // s->regs[REG_INDEX(REG_RSTSR)] = REG_RSTSR_RST;
    // s->regs[REG_INDEX(REG_PWRCON2)] = REG_PWRCON2_RST;
    // // s->regs[REG_INDEX(REG_UNK5)] = REG_UNK5_RST;
    // // s->regs[REG_INDEX(REG_UNK6)] = REG_UNK6_RST;
    // // s->regs[REG_INDEX(REG_UNK7)] = REG_UNK7_RST;
    // s->regs[REG_INDEX(REG_PWRCON3)] = REG_PWRCON3_RST;
    // s->regs[REG_INDEX(REG_PWRCON4)] = REG_PWRCON4_RST;
}

static void s5l8702_timer_init(Object *obj)
{
    S5L8702TimerState *s = S5L8702_TIMER(obj);

    printf("s5l8702_timer_init\n");

    /* Memory mapping */
    memory_region_init_io(&s->iomem, OBJECT(s), &s5l8702_timer_ops, s, TYPE_S5L8702_TIMER, S5L8702_TIMER_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);
}

static void s5l8702_timer_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = s5l8702_timer_reset;
}

static const TypeInfo s5l8702_timer_types[] = {
    {
        .name = TYPE_S5L8702_TIMER,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_init = s5l8702_timer_init,
        .instance_size = sizeof(S5L8702TimerState),
        .class_init = s5l8702_timer_class_init,
    },
};
DEFINE_TYPES(s5l8702_timer_types);
