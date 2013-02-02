#include "hw/hw.h"
#include "hw/boards.h"

const VMStateDescription vmstate_lm32_cpu = {
    .name = "cpu",
    .version_id = 1,
    .minimum_version_id = 1,
    .minimum_version_id_old = 1,
    .fields      = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(env.regs, LM32CPU, 32),
        VMSTATE_UINT32(env.pc, LM32CPU),
        VMSTATE_UINT32(env.ie, LM32CPU),
        VMSTATE_UINT32(env.icc, LM32CPU),
        VMSTATE_UINT32(env.dcc, LM32CPU),
        VMSTATE_UINT32(env.cc, LM32CPU),
        VMSTATE_UINT32(env.eba, LM32CPU),
        VMSTATE_UINT32(env.dc, LM32CPU),
        VMSTATE_UINT32(env.deba, LM32CPU),
        VMSTATE_UINT32_ARRAY(env.bp, LM32CPU, 4),
        VMSTATE_UINT32_ARRAY(env.wp, LM32CPU, 4),
        VMSTATE_END_OF_LIST()
    }
};
