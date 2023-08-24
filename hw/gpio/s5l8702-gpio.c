#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/gpio/s5l8702-gpio.h"

#define PCON(port)  (0x00000000 + (port << 5))
#define PDAT(port)  (0x00000004 + (port << 5))
#define GPIOCMD     0x00000200

static uint64_t s5l8702_gpio_read(void *opaque, hwaddr offset,
                                      unsigned size)
{
    const S5L8702GpioState *s = S5L8702_GPIO(opaque);
    const uint32_t port = offset >> 5;

    uint8_t r = 0;

    switch (offset) {
    case PCON(0):
    case PCON(1):
    case PCON(2):
    case PCON(3):
    case PCON(4):
    case PCON(5):
    case PCON(6):
    case PCON(7):
    case PCON(8):
    case PCON(9):
    case PCON(10):
    case PCON(11):
    case PCON(12):
    case PCON(13):
    case PCON(14):
    case PCON(15):
        r = s->pcon[port];
        break;
    case PDAT(0):
    case PDAT(1):
    case PDAT(2):
    case PDAT(3):
    case PDAT(4):
    case PDAT(5):
    case PDAT(6):
    case PDAT(7):
    case PDAT(8):
    case PDAT(9):
    case PDAT(10):
    case PDAT(11):
    case PDAT(12):
    case PDAT(13):
    case PDAT(14):
    case PDAT(15):
        r = s->pdat[port];
        break;
    // case GPIOCMD:

    //     break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented read (offset 0x%04x)\n",
                      __func__, (uint32_t) offset);
    }

    // return s->regs[idx];
    return 0;
}

static void s5l8702_gpio_write(void *opaque, hwaddr offset,
                                   uint64_t val, unsigned size)
{
    S5L8702GpioState *s = S5L8702_GPIO(opaque);
    const uint32_t port = offset >> 5;

    switch (offset) {
    case PCON(0):
    case PCON(1):
    case PCON(2):
    case PCON(3):
    case PCON(4):
    case PCON(5):
    case PCON(6):
    case PCON(7):
    case PCON(8):
    case PCON(9):
    case PCON(10):
    case PCON(11):
    case PCON(12):
    case PCON(13):
    case PCON(14):
    case PCON(15):
        printf("s5l8702_gpio_write: PCON (port %d) = 0x%08x\n", port, (uint32_t) val);
        s->pcon[port] = (uint8_t) val;
        break;
    case PDAT(0):
    case PDAT(1):
    case PDAT(2):
    case PDAT(3):
    case PDAT(4):
    case PDAT(5):
    case PDAT(6):
    case PDAT(7):
    case PDAT(8):
    case PDAT(9):
    case PDAT(10):
    case PDAT(11):
    case PDAT(12):
    case PDAT(13):
    case PDAT(14):
    case PDAT(15):
        printf("s5l8702_gpio_write: PDAT (port %d) = 0x%08x\n", port, (uint32_t) val);
        s->pdat[port] = (uint8_t) val;
        for (int i = 0; i < 8; i++) {
            if (port == 0 && i == 0) {
                printf("s5l8702_gpio_write: setting CS pin to %d\n", (s->pdat[port] >> i) & 1);
            }
            qemu_set_irq(s->output[port * 8 + i], (s->pdat[port] >> i) & 1);
        }
        break;
    // case GPIOCMD:

    //     break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented write (offset 0x%04x, value 0x%08x)\n",
                      __func__, (uint32_t) offset, (uint32_t) val);
    }
}

static const MemoryRegionOps s5l8702_gpio_ops = {
    .read = s5l8702_gpio_read,
    .write = s5l8702_gpio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    // .valid = {
    //     .min_access_size = 4,
    //     .max_access_size = 4,
    // },
    // .impl.min_access_size = 4,
};

static void s5l8702_gpio_set(void *opaque, int n, int level)
{
    S5L8702GpioState *s = S5L8702_GPIO(opaque);
    const uint32_t port = S5L8702_GPIO_PORT(n);

    if (level) {
        s->pdat[port] |= (1 << S5L8702_GPIO_PIN(n));
    } else {
        s->pdat[port] &= ~(1 << S5L8702_GPIO_PIN(n));
    }
}

static void s5l8702_gpio_reset(DeviceState *dev)
{
    S5L8702GpioState *s = S5L8702_GPIO(dev);

    printf("s5l8702_gpio_reset\n");

    /* Reset registers */
    // memset(s->regs, 0, sizeof(s->regs));

    /* Set default values for registers */

}

static void s5l8702_gpio_init(Object *obj)
{
    S5L8702GpioState *s = S5L8702_GPIO(obj);

    printf("s5l8702_gpio_init\n");

    /* Memory mapping */
    memory_region_init_io(&s->iomem, OBJECT(s), &s5l8702_gpio_ops, s, TYPE_S5L8702_GPIO, S5L8702_GPIO_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    qdev_init_gpio_in(DEVICE(s), s5l8702_gpio_set, S5L8702_GPIO_PINS);
    qdev_init_gpio_out(DEVICE(s), s->output, S5L8702_GPIO_PINS);
}

static void s5l8702_gpio_realize(DeviceState *dev, Error **errp)
{
    S5L8702GpioState *s = S5L8702_GPIO(dev);

    printf("s5l8702_gpio_realize\n");
}

static void s5l8702_gpio_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->realize = s5l8702_gpio_realize;
    dc->reset = s5l8702_gpio_reset;
}

static const TypeInfo s5l8702_gpio_types[] = {
    {
        .name = TYPE_S5L8702_GPIO,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_init = s5l8702_gpio_init,
        .instance_size = sizeof(S5L8702GpioState),
        .class_init = s5l8702_gpio_class_init,
    },
};
DEFINE_TYPES(s5l8702_gpio_types);
