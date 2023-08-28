#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "qemu/log.h"
#include "cpu.h"
#include "exec/address-spaces.h"
#include "hw/boards.h"
#include "hw/qdev-properties.h"
#include "hw/qdev-core.h"
#include "hw/arm/s5l8702.h"
#include "hw/misc/unimp.h"
#include "qemu/units.h"

static void s5l8702_init(Object *obj)
{
    S5L8702State *s = S5L8702(obj);

    printf("s5l8702_init\n");

    object_initialize_child(obj, "cpu", &(s->cpu), ARM_CPU_TYPE_NAME("arm926"));

    for (uint32_t i = 0; i < ARRAY_SIZE(s->vic); i++) {
        object_initialize_child(obj, "vic[*]", &s->vic[i], TYPE_PL192);
    }

    object_initialize_child(obj, "clk", &s->clk, TYPE_S5L8702_CLK);
    object_initialize_child(obj, "aes", &s->aes, TYPE_S5L8702_AES);
    object_initialize_child(obj, "sha", &s->sha, TYPE_S5L8702_SHA);
    object_initialize_child(obj, "gpio", &s->gpio, TYPE_S5L8702_GPIO);

    for (uint32_t i = 0; i < ARRAY_SIZE(s->spi); i++) {
        object_initialize_child(obj, "spi[*]", &s->spi[i], TYPE_S5L8702_SPI);
    }
}

static void s5l8702_realize(DeviceState *dev, Error **errp)
{
    S5L8702State *s = S5L8702(dev);
    MemoryRegion *system_memory = get_system_memory();

    printf("s5l8702_realize\n");

    qdev_realize(DEVICE(&s->cpu), NULL, &error_fatal);

    /* VIC */
    for (int i = 0; i < ARRAY_SIZE(s->vic); i++) {
        sysbus_realize(SYS_BUS_DEVICE(&s->vic[i]), &error_fatal);
        sysbus_mmio_map(SYS_BUS_DEVICE(&s->vic[i]), 0, S5L8702_VIC_BASE_ADDR + (i * 0x1000));
        // sysbus_connect_irq(SYS_BUS_DEVICE(&s->vic[i]), 0, cpu_irq[0]);
        // sysbus_connect_irq(SYS_BUS_DEVICE(&s->vic[i]), 1, cpu_fiq[0]);
    }

    /* CLK */
    sysbus_realize(SYS_BUS_DEVICE(&s->clk), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->clk), 0, S5L8702_CLK_BASE_ADDR);

    /* AES */
    sysbus_realize(SYS_BUS_DEVICE(&s->aes), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->aes), 0, S5L8702_AES_BASE);

    /* SHA */
    sysbus_realize(SYS_BUS_DEVICE(&s->sha), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->sha), 0, S5L8702_SHA_BASE);

    /* GPIO */
    sysbus_realize(SYS_BUS_DEVICE(&s->gpio), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->gpio), 0, S5L8702_GPIO_BASE);

    /* SPI */
    for (uint32_t i = 0; i < ARRAY_SIZE(s->spi); i++) {
        sysbus_realize(SYS_BUS_DEVICE(&s->spi[i]), &error_fatal);
    }
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi[0]), 0, S5L8702_SPI0_BASE);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi[1]), 0, S5L8702_SPI1_BASE);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->spi[2]), 0, S5L8702_SPI2_BASE);

    /* BootROM */
    memory_region_init_ram(&s->brom, OBJECT(dev), "s5l8702.bootrom", S5L8702_BOOTROM_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, S5L8702_BOOTROM_BASE_ADDR, &s->brom);
    memory_region_init_alias(&s->brom_alias, OBJECT(dev), "s5l8702.bootrom-alias", &s->brom, 0, S5L8702_BOOTROM_SIZE);
    memory_region_add_subregion(system_memory, S5L8702_BASE_BOOT_ADDR, &s->brom_alias);

    /* IRAM0 */
    memory_region_init_ram(&s->iram0, OBJECT(dev), "s5l8702.iram0", S5L8702_IRAM0_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, S5L8702_IRAM0_BASE_ADDR, &s->iram0);

    /* IRAM1 */
    memory_region_init_ram(&s->iram1, OBJECT(dev), "s5l8702.iram1", S5L8702_IRAM1_SIZE, &error_fatal);
    memory_region_add_subregion(system_memory, S5L8702_IRAM1_BASE_ADDR, &s->iram1);

    create_unimplemented_device("unimplemented-sfr", 0x38000000, 0x3FFFFFFF); // SFRs
    create_unimplemented_device("wdt", 0x3c800000, 0x100000);
}

static void s5l8702_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    printf("s5l8702_class_init\n");

    dc->realize = s5l8702_realize;
}

static const TypeInfo s5l8702_types[] = {
    {
        .name = TYPE_S5L8702,
        .parent = TYPE_SYS_BUS_DEVICE,
        .instance_size = sizeof(S5L8702State),
        .instance_init = s5l8702_init,
        .class_init = s5l8702_class_init,
    },
};
DEFINE_TYPES(s5l8702_types);
