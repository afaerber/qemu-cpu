/*
 * QEMU UniCore32 CPU
 *
 * Copyright (c) 2010-2011 GUAN Xue-tao
 * Copyright (c) 2012 SUSE LINUX Products GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "cpu-qom.h"
#include "qemu-common.h"

/* CPU models */

typedef struct UniCore32CPUInfo {
    const char *name;
    uint32_t cp0_c0_cpuid;
    uint32_t cp0_c0_cachetype;
    uint32_t cp0_c1_sys;
    uint32_t features;
} UniCore32CPUInfo;

#define UC32_FEATURE(feature) (1u << feature)

static const UniCore32CPUInfo uc32_cpus[] = {
    {
        .name = "UniCore-II",
        .cp0_c0_cpuid = 0x40010863,
        .cp0_c0_cachetype = 0x1dd20d2,
        .cp0_c1_sys = 0x00090078,
        .features = UC32_FEATURE(UC32_HWCAP_CMOV) |
                    UC32_FEATURE(UC32_HWCAP_UCF64),
    },
    {
        .name = "any",
        .cp0_c0_cpuid = 0xffffffff,
        .features = UC32_FEATURE(UC32_HWCAP_CMOV) |
                    UC32_FEATURE(UC32_HWCAP_UCF64),
    }
};

static void uc32_cpu_initfn(Object *obj)
{
    UniCore32CPU *cpu = UNICORE32_CPU(obj);
    UniCore32CPUClass *klass = UNICORE32_CPU_GET_CLASS(cpu);
    CPUUniCore32State *env = &cpu->env;

    memset(env, 0, sizeof(CPUUniCore32State));
    cpu_exec_init(env);
    env->cpu_model_str = object_get_typename(obj);
    env->cp0.c0_cpuid = klass->cp0.c0_cpuid;
    env->cp0.c0_cachetype = klass->cp0.c0_cachetype;
    env->cp0.c1_sys = klass->cp0.c1_sys;
    env->features = klass->features;

    env->uncached_asr = ASR_MODE_USER;
    env->regs[31] = 0;

    tlb_flush(env, 1);
}

static void uc32_cpu_class_init(ObjectClass *klass, void *data)
{
    UniCore32CPUClass *k = UNICORE32_CPU_CLASS(klass);
    const UniCore32CPUInfo *info = data;

    k->cp0.c0_cpuid = info->cp0_c0_cpuid;
    k->cp0.c0_cachetype = info->cp0_c0_cachetype;
    k->cp0.c1_sys = info->cp0_c1_sys;
    k->features = info->features;
}

static void uc32_register_cpu(const UniCore32CPUInfo *info)
{
    TypeInfo type = {
        .name = info->name,
        .parent = TYPE_UNICORE32_CPU,
        .instance_size = sizeof(UniCore32CPU),
        .class_size = sizeof(UniCore32CPUClass),
        .class_init = uc32_cpu_class_init,
        .class_data = (void *)info,
    };

    type_register_static(&type);
}

static const TypeInfo uc32_cpu_info = {
    .name = TYPE_UNICORE32_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(UniCore32CPU),
    .instance_init = uc32_cpu_initfn,
    .abstract = true,
    .class_size = sizeof(UniCore32CPUClass),
};

static void uc32_cpu_register_types(void)
{
    int i;

    type_register_static(&uc32_cpu_info);
    for (i = 0; i < ARRAY_SIZE(uc32_cpus); i++) {
        uc32_register_cpu(&uc32_cpus[i]);
    }
}

type_init(uc32_cpu_register_types)
