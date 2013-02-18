#include "qemu-common.h"
#include "migration/vmstate.h"

const VMStateDescription vmstate_dummy = {};

int vmstate_register_with_alias_id(DeviceState *dev,
                                   int instance_id,
                                   const VMStateDescription *vmsd,
                                   void *base, int alias_id,
                                   int required_for_version)
{
    return 0;
}

int vmstate_register(DeviceState *dev, int instance_id,
                     const VMStateDescription *vmsd, void *base)
{
    return vmstate_register_with_alias_id(dev, instance_id, vmsd, base,
                                          -1, 0);
}

void vmstate_unregister(DeviceState *dev,
                        const VMStateDescription *vmsd,
                        void *opaque)
{
}
