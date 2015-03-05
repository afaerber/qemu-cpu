/*
 * x86 CPU core abstraction
 *
 * Copyright (c) 2015 SUSE Linux GmbH
 */
#ifndef HW_I386_CPU_CORE_H
#define HW_I386_CPU_CORE_H

#include "hw/qdev.h"
#include "cpu.h"

#ifdef TARGET_X86_64
#define TYPE_X86_CPU_CORE "x86_64-cpu-core"
#else
#define TYPE_X86_CPU_CORE "i386-cpu-core"
#endif

#define X86_CPU_CORE(obj) \
    OBJECT_CHECK(X86CPUCore, (obj), TYPE_X86_CPU_CORE)

typedef struct X86CPUCore {
    /*< private >*/
    DeviceState parent_obj;
    /*< public >*/

    X86CPU thread[0];
} X86CPUCore;

#endif
