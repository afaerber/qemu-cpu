/*
 * CPU socket abstraction
 *
 * Copyright (c) 2013-2014 SUSE LINUX Products GmbH
 * Copyright (c) 2015 SUSE Linux GmbH
 */

#include "hw/cpu/socket.h"

static const TypeInfo cpu_socket_type_info = {
    .name = TYPE_CPU_SOCKET,
    .parent = TYPE_DEVICE,
    .abstract = true,
};

static void cpu_socket_register_types(void)
{
    type_register_static(&cpu_socket_type_info);
}

type_init(cpu_socket_register_types)
