#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qemu/cutils.h"
#include "qapi/error.h"
#include "hw/arm/boot.h"
#include "hw/registerfields.h"
#include "qemu/error-report.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/arm/boot.h"
#include "qom/object.h"
#include "hw/arm/ipod-classic.h"
#include "hw/loader.h"
#include "hw/irq.h"
#include "hw/qdev-properties.h"

static char *ipod_classic_get_nand_path(Object *obj, Error **errp)
{
    IpodClassicState *s = IPOD_CLASSIC_MACHINE(obj);
    return g_strdup(s->nand_path);
}

static void ipod_classic_set_nand_path(Object *obj, const char *value, Error **errp)
{
    IpodClassicState *s = IPOD_CLASSIC_MACHINE(obj);
    g_free(s->nand_path);
    s->nand_path = g_strdup(value);
}

static char *ipod_classic_get_bootrom_path(Object *obj, Error **errp)
{
    IpodClassicState *s = IPOD_CLASSIC_MACHINE(obj);
    return g_strdup(s->bootrom_path);
}

static void ipod_classic_set_bootrom_path(Object *obj, const char *value, Error **errp)
{
    IpodClassicState *s = IPOD_CLASSIC_MACHINE(obj);
    g_free(s->bootrom_path);
    s->bootrom_path = g_strdup(value);
}

static char *ipod_classic_get_bootloader_path(Object *obj, Error **errp)
{
    IpodClassicState *s = IPOD_CLASSIC_MACHINE(obj);
    return g_strdup(s->bootloader_path);
}

static void ipod_classic_set_bootloader_path(Object *obj, const char *value, Error **errp)
{
    IpodClassicState *s = IPOD_CLASSIC_MACHINE(obj);
    g_free(s->bootloader_path);
    s->bootloader_path = g_strdup(value);
}

static void ipod_classic_init(Object *obj)
{
    MachineState *machine = MACHINE(obj);
    IpodClassicState *s = IPOD_CLASSIC_MACHINE(obj);
    
    printf("ipod_classic_init\n");

    if (!object_property_add_str(obj, "nand", ipod_classic_get_nand_path, ipod_classic_set_nand_path)) {
        printf("ipod_classic_init: failed to add nand property\n");
    }

    if (!object_property_add_str(obj, "bootrom", ipod_classic_get_bootrom_path, ipod_classic_set_bootrom_path)) {
        printf("ipod_classic_init: failed to add bootrom property\n");
    }

    if (!object_property_add_str(obj, "bootloader", ipod_classic_get_bootloader_path, ipod_classic_set_bootloader_path)) {
        printf("ipod_classic_init: failed to add bootloader property\n");
    }
}

// static void ipod_classic_reset(void *opaque)
// {
//     IpodClassicState *s = IPOD_CLASSIC_MACHINE(opaque);
    
//     printf("ipod_classic_reset\n");
    
//     cpu_reset(CPU(&(s->soc.cpu)));
//     cpu_set_pc(CPU(&(s->soc.cpu)), S5L8702_BASE_BOOT_ADDR);
// }

static void ipod_classic_machine_init(MachineState *machine)
{
    IpodClassicState *s = IPOD_CLASSIC_MACHINE(machine);
    
    printf("ipod_classic_machine_init\n");

    /* BIOS is not supported by this board */
    if (machine->firmware) {
        error_report("BIOS not supported for this machine");
        exit(1);
    }

    /* This board has fixed size RAM (64MiB) */
    if (machine->ram_size != 64 * MiB) {
        error_report("This machine can only be used with 64MiB RAM");
        exit(1);
    }

    /* Only allow ARM926 for this board */
    if (strcmp(machine->cpu_type, ARM_CPU_TYPE_NAME("arm926")) != 0) {
        error_report("This board can only be used with arm926 CPU");
        exit(1);
    }

    /* Initialize s5l8702 soc */
    object_initialize_child(OBJECT(s), "soc", &s->soc, TYPE_S5L8702);
    sysbus_realize(SYS_BUS_DEVICE(&s->soc), &error_fatal);

    /* DRAM */
    memory_region_init_ram(&s->dram, OBJECT(s), "ipod_classic.dram", machine->ram_size, &error_fatal);
    memory_region_add_subregion(get_system_memory(), S5L8702_DRAM_BASE_ADDR, &s->dram);

    /* Connect an SPI flash to SPI0 */
    DeviceState *flash_dev = qdev_new("sst25vf080b"); // According to https://freemyipod.org/wiki/Classic_3G
    DriveInfo *dinfo = drive_get(IF_MTD, 0, 0);
    if (dinfo) {
        printf("dinfo found\n");
        qdev_prop_set_drive_err(flash_dev, "drive",
                                blk_by_legacy_dinfo(dinfo),
                                &error_fatal);
    } else {
        printf("No dinfo found\n");
    }
    qdev_realize_and_unref(flash_dev, BUS(s->soc.spi0.spi), &error_fatal);

    qemu_irq flash_cs = qdev_get_gpio_in_named(flash_dev, SSI_GPIO_CS, 0);
    qdev_connect_gpio_out(DEVICE(&(s->soc.gpio)), 0, flash_cs);

    /* Read the bootrom, copy it to memory and execute it */
    assert(s->bootrom_path);

    uint8_t *bootrom = NULL;
    size_t bootrom_size = 0;
    if (g_file_get_contents(s->bootrom_path, (char **) &bootrom, &bootrom_size, NULL)) {
        // copy bootrom into s->soc.brom memoryregion
        AddressSpace *nsas = cpu_get_address_space(CPU(&(s->soc.cpu)), ARMASIdx_NS);
        address_space_write(nsas, 0x20000000, MEMTXATTRS_UNSPECIFIED, bootrom, bootrom_size);
    } else {
        printf("ipod_classic_machine_init: failed to read bootrom\n");
        exit(1);
    }

    // qemu_register_reset(ipod_classic_reset, s);
}

static void ipod_classic_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    mc->init = ipod_classic_machine_init;
    mc->default_cpu_type = ARM_CPU_TYPE_NAME("arm926");
    mc->default_ram_size = 64 * MiB;
    // mc->default_cpus = 1;
    
    printf("ipod_classic_class_init\n");
};

static const TypeInfo ipod_classic_types[] = {
    {
        .name = TYPE_IPOD_CLASSIC_MACHINE,
        .parent = TYPE_MACHINE,
        .instance_size = sizeof(IpodClassicState),
        .instance_init = ipod_classic_init,
        .class_init = ipod_classic_class_init,
    },
};
DEFINE_TYPES(ipod_classic_types)
