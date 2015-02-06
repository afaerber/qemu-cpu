/*
 * x86 CPU socket abstraction
 *
 * Copyright (c) 2015 SUSE Linux GmbH
 */

#include "hw/i386/cpu-socket.h"

static int x86_cpu_socket_realize_child(Object *child, void *opaque)
{
    Error **errp = opaque;
    Error *local_err = NULL;

    object_property_set_bool(child, true, "realized", &local_err);
    error_propagate(errp, local_err);

    return local_err != NULL ? 1 : 0;
}

static void x86_cpu_socket_realize(DeviceState *dev, Error **errp)
{
    /* XXX generic */
    object_child_foreach(OBJECT(dev), x86_cpu_socket_realize_child, errp);
}

static void x86_cpu_socket_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = x86_cpu_socket_realize;
}

static const TypeInfo x86_cpu_socket_type_info = {
    .name = TYPE_X86_CPU_SOCKET,
    .parent = TYPE_CPU_SOCKET,
    .instance_size = sizeof(X86CPUSocket),
    .class_init = x86_cpu_socket_class_init,
};

static void x86_cpu_socket_register_types(void)
{
    type_register_static(&x86_cpu_socket_type_info);
}

type_init(x86_cpu_socket_register_types)
