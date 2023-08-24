#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/ssi/s5l8702-spi.h"

#define SPICTRL     0x00
#define SPISETUP    0x04
#define SPISTATUS   0x08
#define SPIPIN      0x0c
#define SPITXDATA   0x10
#define SPIRXDATA   0x20
#define SPICLKDIV   0x30
#define SPIRXLIMIT  0x34

#define REG_INDEX(offset)   (offset / sizeof(uint32_t))

#define FIFO_CAPACITY   1 // 8

static void s5l8702_spi_update_irq(S5L8702SpiState *s)
{
    int level;

    // if (fifo8_num_used(&s->tx_fifo) < s->regs[R_TXMARK]) {
    //     s->regs[R_IP] |= IP_TXWM;
    // } else {
    //     s->regs[R_IP] &= ~IP_TXWM;
    // }

    // if (fifo8_num_used(&s->rx_fifo) > s->regs[R_RXMARK]) {
    //     s->regs[R_IP] |= IP_RXWM;
    // } else {
    //     s->regs[R_IP] &= ~IP_RXWM;
    // }

    // level = s->regs[R_IP] & s->regs[R_IE] ? 1 : 0;
    // qemu_set_irq(s->irq, level);
}

static uint64_t s5l8702_spi_read(void *opaque, hwaddr offset,
                                      unsigned size)
{
    S5L8702SpiState *s = S5L8702_SPI(opaque);
    const uint32_t idx = REG_INDEX(offset);

    uint64_t r = 0;

    switch (offset) {
    case SPISTATUS:
        if (s->has_tx_data) {
            r &= ~0x1f0;
            r |= 0x100;
        }
        if (s->has_rx_data || (s->spisetup & 1)) {
            r |= 0x3e00;
        }
        // printf("%s: SPISTATUS: 0x%08x\n", __func__, (uint32_t) r);
        break;
    case SPISETUP:
        r = s->spisetup;
        printf("%s: SPISETUP: 0x%08x\n", __func__, (uint32_t) r);
        break;
    case SPITXDATA:
        r = s->spitxdata;
        printf("%s: SPITXDATA: 0x%08x\n", __func__, (uint32_t) r);
        break;
    case SPIRXDATA:
        s->has_rx_data = false;
        if (s->spisetup & 1) {
            s->spirxdata = ssi_transfer(s->spi, 0x000000FF);
        }
        r = s->spirxdata;
        // printf("%s: SPIRXDATA: 0x%08x\n", __func__, (uint32_t) r);
        break;
    // case SPICTRL:
    // case SPISETUP:
    // case SPISTATUS:
    // case SPIPIN:
    // case SPITXDATA:
    // case SPIRXDATA:
    // case SPICLKDIV:
    // case SPIRXLIMIT:
    //     break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented read (offset 0x%04x)\n",
                      __func__, (uint32_t) offset);
        break;
    }

    s5l8702_spi_update_irq(s);

    return r;
}

static void s5l8702_spi_write(void *opaque, hwaddr offset,
                                   uint64_t val, unsigned size)
{
    S5L8702SpiState *s = S5L8702_SPI(opaque);
    const uint32_t idx = REG_INDEX(offset);

    switch (offset) {
    // case SPIRXLIMIT:
    //     printf("%s: SPIRXLIMIT: 0x%08x\n", __func__, (uint32_t) val);
    //     break;
    case SPISETUP:
        printf("%s: SPISETUP: 0x%08x\n", __func__, (uint32_t) val);
        s->spisetup = (uint32_t) val;
        break;
    case SPITXDATA:
        printf("%s: SPITXDATA: 0x%08x\n", __func__, (uint32_t) val);

        s->spitxdata = (uint32_t) val;
        s->has_tx_data = true;

        s->spirxdata = ssi_transfer(s->spi, s->spitxdata);

        s->has_tx_data = false;
        s->has_rx_data = true;
        break;
    default:
        qemu_log_mask(LOG_UNIMP, "%s: unimplemented write (offset 0x%04x, value 0x%08x)\n",
                      __func__, (uint32_t) offset, (uint32_t) val);
        break;
    }

    s5l8702_spi_update_irq(s);
}

static const MemoryRegionOps s5l8702_spi_ops = {
    .read = s5l8702_spi_read,
    .write = s5l8702_spi_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    // .valid = {
    //     .min_access_size = 4,
    //     .max_access_size = 4,
    // },
    // .impl.min_access_size = 4,
};

static void s5l8702_spi_reset(DeviceState *dev)
{
    S5L8702SpiState *s = S5L8702_SPI(dev);

    printf("s5l8702_spi_reset\n");

    /* Reset registers */
    // memset(s->regs, 0, sizeof(s->regs));

    /* Set default values for registers */

}

static void s5l8702_spi_init(Object *obj)
{
    S5L8702SpiState *s = S5L8702_SPI(obj);

    printf("s5l8702_spi_init\n");

    /* Memory mapping */
    memory_region_init_io(&s->iomem, OBJECT(s), &s5l8702_spi_ops, s, TYPE_S5L8702_SPI, S5L8702_SPI_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    s->spi = ssi_create_bus(DEVICE(obj), "spi");
}

static void s5l8702_spi_realize(DeviceState *dev, Error **errp)
{
    S5L8702SpiState *s = S5L8702_SPI(dev);

    printf("s5l8702_spi_realize\n");

}

static void s5l8702_spi_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = s5l8702_spi_realize;
    dc->reset = s5l8702_spi_reset; // TODO: Fix deprecation
}

static const TypeInfo s5l8702_spi_types[] = {
    {
        .name = TYPE_S5L8702_SPI,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_size = sizeof(S5L8702SpiState),
        .instance_init = s5l8702_spi_init,
        .class_init = s5l8702_spi_class_init,
    },
};
DEFINE_TYPES(s5l8702_spi_types);
