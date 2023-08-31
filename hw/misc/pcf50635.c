#include "qemu/osdep.h"
#include "qemu/units.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/misc/pcf50635.h"

/* OOC */
#define PCF50635_VERSION    0x00
#define PCF50635_VARIANT    0x01
#define PCF50635_OOCSHDWN   0x0C
#define PCF50635_OOCWAKE    0x0D
#define PCF50635_OOCTIM1    0x0E
#define PCF50635_OOCTIM2    0x0F
#define PCF50635_OOCMODE    0x10
#define PCF50635_OOCCTL     0x11
#define PCF50635_OOCSTAT    0x12

/* INT */
#define PCF50635_INT1       0x02
#define PCF50635_INT2       0x03
#define PCF50635_INT3       0x04
#define PCF50635_INT4       0x05
#define PCF50635_INT5       0x06
#define PCF50635_INT1MASK   0x07
#define PCF50635_INT2MASK   0x08
#define PCF50635_INT3MASK   0x09
#define PCF50635_INT4MASK   0x0A
#define PCF50635_INT5MASK   0x0B

/* GPIO */
#define PCF50635_GPIOCTL    0x13
#define PCF50635_GPIO1CFG   0x14
#define PCF50635_GPIO2CFG   0x15
#define PCF50635_GPIO3CFG   0x16

/* GPO */
#define PCF50635_GPOCFG     0x17

/* SVM */
#define PCF50635_SVMCTL     0x19

/* BVM */
#define PCF50635_BVMCTL     0x18

/* PSM */
#define PCF50635_STBYCTL1   0x3B
#define PCF50635_STBYCTL2   0x3C
#define PCF50635_DEBPF1     0x3D
#define PCF50635_DEBPF2     0x3E
#define PCF50635_DEBPF3     0x3F
#define PCF50635_DCDCSTAT   0x41
#define PCF50635_LDOSTAT    0x42



static int pcf50635_event(I2CSlave *slave, enum i2c_event event)
{
    Pcf50635State *s = PCF50635(slave);
    
    printf("pcf50635_event: %d\n", event);
    
    return 0;
}

static uint8_t pcf50635_recv(I2CSlave *slave)
{
    Pcf50635State *s = PCF50635(slave);
    uint8_t r = 0;

    switch (s->cmd) {

    default:
        printf("pcf50635_recv: unknown cmd %02x\n", s->cmd);
        break;
    }
    
    return r;
}

static int pcf50635_send(I2CSlave *slave, uint8_t data)
{
    Pcf50635State *s = PCF50635(slave);
    
    printf("pcf50635_send: %02x\n", data);

    return 0;
}

static void pcf50635_reset(DeviceState *dev)
{
    Pcf50635State *s = PCF50635(dev);

    printf("pcf50635_reset\n");

    /* Reset registers */
    memset(s->regs, 0, sizeof(s->regs));
}

static void pcf50635_class_init(ObjectClass *klass, void *data)
{
    ResettableClass *rc = RESETTABLE_CLASS(klass);
    DeviceClass *dc = DEVICE_CLASS(klass);
    I2CSlaveClass *isc = I2C_SLAVE_CLASS(klass);

    dc->reset = pcf50635_reset;
    // rc->phases.enter = pcf50635_reset_enter;
    // dc->vmsd = &vmstate_pcf50635;
    isc->event = pcf50635_event;
    isc->recv = pcf50635_recv;
    isc->send = pcf50635_send;
}

static const TypeInfo pcf50635_types[] = {
    {
        .name = TYPE_PCF50635,
        .parent = TYPE_I2C_SLAVE,
        .instance_size = sizeof(Pcf50635State),
        .class_init = pcf50635_class_init,
    },
};
DEFINE_TYPES(pcf50635_types);
