/*
 * Arm PrimeCell PL192 Vector Interrupt Controller
 *
 * Copyright (c) 2006 CodeSourcery.
 * Written by Paul Brook
 *
 * This code is licensed under the GPL.
 */

#include "qemu/osdep.h"
#include "hw/irq.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "qom/object.h"
#include "hw/intc/pl192.h"

static const unsigned char pl192_id[] =
{ 0x92, 0x11, 0x04, 0x00, 0x0D, 0xf0, 0x05, 0xb1 };

static inline uint32_t pl192_irq_level(PL192State *s)
{
    return (s->level | s->soft_level) & s->irq_enable & ~s->fiq_select;
}

/* Update interrupts. */
static void pl192_update(PL192State *s)
{
    uint32_t level = pl192_irq_level(s);
    int set;

    set = (level & s->prio_mask[s->priority]) != 0;
    qemu_set_irq(s->irq, set);
    set = ((s->level | s->soft_level) & s->fiq_select) != 0;
    qemu_set_irq(s->fiq, set);
}

static void pl192_set_irq(void *opaque, int irq, int level)
{
    PL192State *s = (PL192State *)opaque;

    if (level)
        s->level |= 1u << irq;
    else
        s->level &= ~(1u << irq);
    pl192_update(s);
}

static void pl192_update_vectors(PL192State *s)
{
    uint32_t mask;
    int i;
    int n;

    mask = 0;
    for (i = 0; i < 16; i++)
      {
        s->prio_mask[i] = mask;
        if (s->vect_control[i] & 0x20)
          {
            n = s->vect_control[i] & 0x1f;
            mask |= 1 << n;
          }
      }
    s->prio_mask[16] = mask;
    pl192_update(s);
}

static uint64_t pl192_read(void *opaque, hwaddr offset,
                           unsigned size)
{
    PL192State *s = (PL192State *) opaque;
    int i;

    if (offset >= 0xfe0 && offset < 0x1000) {
        return pl192_id[(offset - 0xfe0) >> 2];
    }
    if (offset >= 0x100 && offset < 0x140) {
        return s->vect_addr[(offset - 0x100) >> 2];
    }
    if (offset >= 0x200 && offset < 0x240) {
        return s->vect_control[(offset - 0x200) >> 2];
    }
    switch (offset >> 2) {
    case 0: /* IRQSTATUS */
        return pl192_irq_level(s);
    case 1: /* FIQSATUS */
        return (s->level | s->soft_level) & s->fiq_select;
    case 2: /* RAWINTR */
        return s->level | s->soft_level;
    case 3: /* INTSELECT */
        return s->fiq_select;
    case 4: /* INTENABLE */
        return s->irq_enable;
    case 6: /* SOFTINT */
        return s->soft_level;
    case 8: /* PROTECTION */
        return s->protected;
    case 12: /* VECTADDR */
        /* Read vector address at the start of an ISR.  Increases the
         * current priority level to that of the current interrupt.
         *
         * Since an enabled interrupt X at priority P causes prio_mask[Y]
         * to have bit X set for all Y > P, this loop will stop with
         * i == the priority of the highest priority set interrupt.
         */
        for (i = 0; i < s->priority; i++) {
            if ((s->level | s->soft_level) & s->prio_mask[i + 1]) {
                break;
            }
        }

        /* Reading this value with no pending interrupts is undefined.
           We return the default address.  */
        if (i == PL192_NUM_PRIO)
          return s->vect_addr[16];
        if (i < s->priority)
          {
            s->prev_prio[i] = s->priority;
            s->priority = i;
            pl192_update(s);
          }
        return s->vect_addr[s->priority];
    case 13: /* DEFVECTADDR */
        return s->vect_addr[16];
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                      "pl192_read: Bad offset %x\n", (int)offset);
        return 0;
    }
}

static void pl192_write(void *opaque, hwaddr offset,
                        uint64_t val, unsigned size)
{
    PL192State *s = (PL192State *)opaque;

    if (offset >= 0x100 && offset < 0x140) {
        s->vect_addr[(offset - 0x100) >> 2] = val;
        pl192_update_vectors(s);
        return;
    }
    if (offset >= 0x200 && offset < 0x240) {
        s->vect_control[(offset - 0x200) >> 2] = val;
        pl192_update_vectors(s);
        return;
    }
    switch (offset >> 2) {
    case 0: /* SELECT */
        /* This is a readonly register, but linux tries to write to it
           anyway.  Ignore the write.  */
        break;
    case 3: /* INTSELECT */
        s->fiq_select = val;
        break;
    case 4: /* INTENABLE */
        s->irq_enable |= val;
        break;
    case 5: /* INTENCLEAR */
        s->irq_enable &= ~val;
        break;
    case 6: /* SOFTINT */
        s->soft_level |= val;
        break;
    case 7: /* SOFTINTCLEAR */
        s->soft_level &= ~val;
        break;
    case 8: /* PROTECTION */
        /* TODO: Protection (supervisor only access) is not implemented.  */
        s->protected = val & 1;
        break;
    case 12: /* VECTADDR */
        /* Restore the previous priority level.  The value written is
           ignored.  */
        if (s->priority < PL192_NUM_PRIO)
            s->priority = s->prev_prio[s->priority];
        break;
    case 13: /* DEFVECTADDR */
        s->vect_addr[16] = val;
        break;
    case 0xc0: /* ITCR */
        if (val) {
            qemu_log_mask(LOG_UNIMP, "pl192: Test mode not implemented\n");
        }
        break;
    default:
        qemu_log_mask(LOG_GUEST_ERROR,
                     "pl192_write: Bad offset %x\n", (int)offset);
        return;
    }
    pl192_update(s);
}

static const MemoryRegionOps pl192_ops = {
    .read = pl192_read,
    .write = pl192_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void pl192_reset(DeviceState *d)
{
    PL192State *s = PL192(d);
    int i;

    for (i = 0; i < 16; i++) {
        s->vect_addr[i] = 0;
        s->vect_control[i] = 0;
    }
    s->vect_addr[16] = 0;
    s->prio_mask[17] = 0xffffffff;
    s->priority = PL192_NUM_PRIO;
    pl192_update_vectors(s);
}

static void pl192_init(Object *obj)
{
    DeviceState *dev = DEVICE(obj);
    PL192State *s = PL192(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->iomem, obj, &pl192_ops, s, "pl192", 0x1000);
    sysbus_init_mmio(sbd, &s->iomem);
    qdev_init_gpio_in(dev, pl192_set_irq, 32);
    sysbus_init_irq(sbd, &s->irq);
    sysbus_init_irq(sbd, &s->fiq);
}

static void pl192_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = pl192_reset;
}

static const TypeInfo pl192_types[] = {
    {
        .name = TYPE_PL192,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_size = sizeof(PL192State),
        .instance_init = pl192_init,
        .class_init = pl192_class_init,
    },
};
DEFINE_TYPES(pl192_types);
