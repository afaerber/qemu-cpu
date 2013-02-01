/*
 * QEMU x86 CPU
 *
 * Copyright (c) 2012 SUSE LINUX Products GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/lgpl-2.1.html>
 */
#ifndef QEMU_I386_CPU_QOM_H
#define QEMU_I386_CPU_QOM_H

#include "qom/cpu.h"
#include "cpu.h"
#include "qapi/error.h"

#ifdef TARGET_X86_64
#define TYPE_X86_CPU "x86_64-cpu"
#else
#define TYPE_X86_CPU "i386-cpu"
#endif

#define TYPE_HOST_X86_CPU "host-" TYPE_X86_CPU

typedef struct x86_def_t {
    const char *name;
    uint32_t level;
    /* vendor is zero-terminated, 12 character ASCII string */
    char vendor[CPUID_VENDOR_SZ + 1];
    int family;
    int model;
    int stepping;
    uint32_t features, ext_features, ext2_features, ext3_features;
    uint32_t kvm_features, svm_features;
    uint32_t xlevel;
    char model_id[48];
    /* Store the results of Centaur's CPUID instructions */
    uint32_t ext4_features;
    uint32_t xlevel2;
    /* The feature bits on CPUID[EAX=7,ECX=0].EBX */
    uint32_t cpuid_7_0_ebx_features;
} x86_def_t;

#define X86_CPU_CLASS(klass) \
    OBJECT_CLASS_CHECK(X86CPUClass, (klass), TYPE_X86_CPU)
#define X86_CPU(obj) \
    OBJECT_CHECK(X86CPU, (obj), TYPE_X86_CPU)
#define X86_CPU_GET_CLASS(obj) \
    OBJECT_GET_CLASS(X86CPUClass, (obj), TYPE_X86_CPU)

/**
 * X86CPUClass:
 * @parent_realize: The parent class' realize handler.
 * @parent_reset: The parent class' reset handler.
 * @info: Model-specific data.
 *
 * An x86 CPU model or family.
 */
typedef struct X86CPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    DeviceRealize parent_realize;
    void (*parent_reset)(CPUState *cpu);

    x86_def_t info;
} X86CPUClass;

/**
 * X86CPU:
 * @env: #CPUX86State
 *
 * An x86 CPU.
 */
typedef struct X86CPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUX86State env;
} X86CPU;

static inline X86CPU *x86_env_get_cpu(CPUX86State *env)
{
    return X86_CPU(container_of(env, X86CPU, env));
}

#define ENV_GET_CPU(e) CPU(x86_env_get_cpu(e))


#endif
