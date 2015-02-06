/*
 * x86 CPU socket abstraction
 *
 * Copyright (c) 2015 SUSE Linux GmbH
 */
#ifndef HW_I386_CPU_SOCKET_H
#define HW_I386_CPU_SOCKET_H

#include "hw/cpu/socket.h"
#include "cpu-core.h"

#ifdef TARGET_X86_64
#define TYPE_X86_CPU_SOCKET "x86_64-cpu-socket"
#else
#define TYPE_X86_CPU_SOCKET "i386-cpu-socket"
#endif

#define X86_CPU_SOCKET(obj) \
    OBJECT_CHECK(X86CPUSocket, (obj), TYPE_X86_CPU_SOCKET)

typedef struct X86CPUSocket {
    /*< private >*/
    DeviceState parent_obj;
    /*< public >*/

    X86CPUCore core[0];
} X86CPUSocket;

#endif
