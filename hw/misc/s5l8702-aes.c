#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/misc/s5l8702-aes.h"

#define REG_INDEX(offset) (offset / sizeof(uint32_t))

static uint64_t s5l8702_aes_read(void *opaque, hwaddr offset,
                                      unsigned size)
{
    const S5L8702AesState *s = S5L8702_AES(opaque);
    const uint32_t idx = REG_INDEX(offset);

    switch (offset) {
    case 0xc: 
        break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented read (offset 0x%04x)\n",
                      __func__, (uint32_t) offset);
    }

    return s->regs[idx];
}

static void s5l8702_aes_write(void *opaque, hwaddr offset,
                                   uint64_t val, unsigned size)
{
    S5L8702AesState *s = S5L8702_AES(opaque);
    const uint32_t idx = REG_INDEX(offset);

    assert(0);

    switch (offset) {
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented write (offset 0x%04x, value 0x%08x)\n",
                      __func__, (uint32_t) offset, (uint32_t) val);
    }

    s->regs[idx] = (uint32_t) val;
}

static const MemoryRegionOps s5l8702_aes_ops = {
    .read = s5l8702_aes_read,
    .write = s5l8702_aes_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    // .valid = {
    //     .min_access_size = 4,
    //     .max_access_size = 4,
    // },
    // .impl.min_access_size = 4,
};

static void s5l8702_aes_reset(DeviceState *dev)
{
    S5L8702AesState *s = S5L8702_AES(dev);

    printf("s5l8702_aes_reset\n");

    /* Reset registers */
    memset(s->regs, 0, sizeof(s->regs));

    /* Set default values for registers */
    s->regs[REG_INDEX(0xC)] = 0x0000000F; // simulate AES finished
}

static void s5l8702_aes_init(Object *obj)
{
    S5L8702AesState *s = S5L8702_AES(obj);

    printf("s5l8702_aes_init\n");

    /* Memory mapping */
    memory_region_init_io(&s->iomem, OBJECT(s), &s5l8702_aes_ops, s, TYPE_S5L8702_AES, S5L8702_AES_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);
}

static void s5l8702_aes_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = s5l8702_aes_reset;
}

static const TypeInfo s5l8702_aes_types[] = {
    {
        .name = TYPE_S5L8702_AES,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_init = s5l8702_aes_init,
        .instance_size = sizeof(S5L8702AesState),
        .class_init = s5l8702_aes_class_init,
    },
};
DEFINE_TYPES(s5l8702_aes_types);
