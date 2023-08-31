#ifndef HW_MISC_PCF50635_H
#define HW_MISC_PCF50635_H

#include "qom/object.h"
#include "hw/i2c/i2c.h"

#define TYPE_PCF50635    "pcf50635"
OBJECT_DECLARE_SIMPLE_TYPE(Pcf50635State, PCF50635)

#define PCF50635_NUM_REGS   64

struct Pcf50635State {
    /*< private >*/
    I2CSlave i2c;

    /*< public >*/
    uint8_t cmd;
    uint8_t regs[PCF50635_NUM_REGS];
};

#endif /* HW_MISC_PCF50635_H */
