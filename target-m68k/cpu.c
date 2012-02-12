/*
 * QEMU Motorola 68k CPU
 *
 * Copyright (c) 2006-2007 CodeSourcery
 * Written by Paul Brook
 *
 * Copyright (c) 2012 SUSE LINUX Products GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/lgpl-2.1.html>
 */

#include "cpu-qom.h"
#include "qemu-common.h"

static void m68k_cpu_reset(CPUState *c)
{
    M68kCPU *cpu = M68K_CPU(c);
    M68kCPUClass *klass = M68K_CPU_GET_CLASS(cpu);
    CPUM68KState *env = &cpu->env;

    if (qemu_loglevel_mask(CPU_LOG_RESET)) {
        qemu_log("CPU Reset (CPU %d)\n", env->cpu_index);
        log_cpu_state(env, 0);
    }

    klass->parent_reset(c);

    memset(env, 0, offsetof(CPUM68KState, breakpoints));
#if !defined(CONFIG_USER_ONLY)
    env->sr = 0x2700;
#endif
    m68k_switch_sp(env);
    /* ??? FP regs should be initialized to NaN.  */
    env->cc_op = CC_OP_FLAGS;
    /* TODO: We should set PC from the interrupt vector.  */
    env->pc = 0;
    tlb_flush(env, 1);
}

/* CPU models */

typedef struct M68kCPUInfo {
    const char *name;
    uint32_t features;
} M68kCPUInfo;

#define M68K_FEATURE(feature) (1u << M68K_FEATURE_ ## feature)

static const M68kCPUInfo m68k_cpus[] = {
    {
        .name = "m5206",
        .features = M68K_FEATURE(CF_ISA_A),
    },
    {
        .name = "m5208",
        .features = M68K_FEATURE(CF_ISA_A) |
                    M68K_FEATURE(CF_ISA_APLUSC) |
                    M68K_FEATURE(BRAL) |
                    M68K_FEATURE(CF_EMAC) |
                    M68K_FEATURE(USP),
    },
    {
        .name = "cfv4e",
        .features = M68K_FEATURE(CF_ISA_A) |
                    M68K_FEATURE(CF_ISA_B) |
                    M68K_FEATURE(BRAL) |
                    M68K_FEATURE(CF_FPU) |
                    M68K_FEATURE(CF_EMAC) |
                    M68K_FEATURE(USP),
    },
    {
        .name = "any",
        .features = M68K_FEATURE(CF_ISA_A) |
                    M68K_FEATURE(CF_ISA_APLUSC) |
                    M68K_FEATURE(CF_ISA_B) |
                    M68K_FEATURE(BRAL) |
                    M68K_FEATURE(CF_FPU) |
                    /* MAC and EMAC are mututally exclusive, so pick EMAC.
                       It's mostly backwards compatible. */
                    M68K_FEATURE(CF_EMAC) |
                    M68K_FEATURE(CF_EMAC_B) |
                    M68K_FEATURE(USP) |
                    M68K_FEATURE(EXT_FULL) |
                    M68K_FEATURE(WORD_INDEX),
    },
};


static void m68k_cpu_initfn(Object *obj)
{
    M68kCPU *cpu = M68K_CPU(obj);
    M68kCPUClass *klass = M68K_CPU_GET_CLASS(cpu);
    CPUM68KState *env = &cpu->env;

    memset(env, 0, sizeof(CPUM68KState));
    cpu_exec_init(env);

    env->cpu_model_str = object_get_typename(obj);
    env->features = klass->features;

    cpu_reset(CPU(cpu));
}

static void m68k_cpu_class_init(ObjectClass *klass, void *data)
{
    CPUClass *cpu_class = CPU_CLASS(klass);
    M68kCPUClass *k = M68K_CPU_CLASS(klass);
    const M68kCPUInfo *info = data;

    k->parent_reset = cpu_class->reset;
    cpu_class->reset = m68k_cpu_reset;

    k->features = info->features;
}

static void cpu_register(const M68kCPUInfo *info)
{
    TypeInfo type = {
        .name = info->name,
        .parent = TYPE_M68K_CPU,
        .instance_size = sizeof(M68kCPU),
        .instance_init = m68k_cpu_initfn,
        .class_size = sizeof(M68kCPUClass),
        .class_init = m68k_cpu_class_init,
        .class_data = (void *)info,
    };

    type_register_static(&type);
}

static const TypeInfo m68k_cpu_type_info = {
    .name = TYPE_M68K_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(M68kCPU),
    .abstract = true,
    .class_size = sizeof(M68kCPUClass),
};

static void m68k_cpu_register_types(void)
{
    int i;

    type_register_static(&m68k_cpu_type_info);
    for (i = 0; i < ARRAY_SIZE(m68k_cpus); i++) {
        cpu_register(&m68k_cpus[i]);
    }
}

type_init(m68k_cpu_register_types)
