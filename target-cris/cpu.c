/*
 * QEMU CRIS CPU
 *
 * Copyright (c) 2008 AXIS Communications AB
 * Written by Edgar E. Iglesias.
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

#include "cpu-qom.h"
#include "qemu-common.h"
#include "mmu.h"

static void cris_cpu_reset(CPUState *c)
{
    CRISCPU *cpu = CRIS_CPU(c);
    CRISCPUClass *klass = CRIS_CPU_GET_CLASS(cpu);
    CPUCRISState *env = &cpu->env;

    if (qemu_loglevel_mask(CPU_LOG_RESET)) {
        qemu_log("CPU Reset (CPU %d)\n", env->cpu_index);
        log_cpu_state(env, 0);
    }

    klass->parent_reset(c);

    memset(env, 0, offsetof(CPUCRISState, breakpoints));
    env->pregs[PR_VR] = klass->vr;
    tlb_flush(env, 1);

#if defined(CONFIG_USER_ONLY)
    /* start in user mode with interrupts enabled.  */
    env->pregs[PR_CCS] |= U_FLAG | I_FLAG | P_FLAG;
#else
    cris_mmu_init(env);
    env->pregs[PR_CCS] = 0;
#endif
}

/* CPU models */

typedef struct CRISCPUInfo {
    const char *name;
    uint32_t vr;
} CRISCPUInfo;

static const CRISCPUInfo cris_cpus[] = {
    {
        .name = "crisv8",
        .vr = 8,
    },
    {
        .name = "crisv9",
        .vr = 9,
    },
    {
        .name = "crisv10",
        .vr = 10,
    },
    {
        .name = "crisv11",
        .vr = 11,
    },
    {
        .name = "crisv32",
        .vr = 32,
    },
};

static void cris_cpu_initfn(Object *obj)
{
    CRISCPU *cpu = CRIS_CPU(obj);
    CRISCPUClass *klass = CRIS_CPU_GET_CLASS(cpu);
    CPUCRISState *env = &cpu->env;

    memset(env, 0, sizeof(*env));
    cpu_exec_init(env);
    env->cpu_model_str = object_get_typename(obj);

    env->pregs[PR_VR] = klass->vr;

    cpu_reset(CPU(cpu));
}

static void cris_cpu_class_init(ObjectClass *klass, void *data)
{
    CPUClass *cpu_class = CPU_CLASS(klass);
    CRISCPUClass *k = CRIS_CPU_CLASS(klass);
    const CRISCPUInfo *info = data;

    k->parent_reset = cpu_class->reset;
    cpu_class->reset = cris_cpu_reset;

    k->vr = info->vr;
}

static void cpu_register(const CRISCPUInfo *info)
{
    TypeInfo type = {
        .name = info->name,
        .parent = TYPE_CRIS_CPU,
        .instance_size = sizeof(CRISCPU),
        .instance_init = cris_cpu_initfn,
        .class_size = sizeof(CRISCPUClass),
        .class_init = cris_cpu_class_init,
        .class_data = (void *)info,
    };

    type_register_static(&type);
}

static const TypeInfo cris_cpu_type_info = {
    .name = TYPE_CRIS_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(CRISCPU),
    .abstract = true,
    .class_size = sizeof(CRISCPUClass),
};

static void cris_cpu_register_types(void)
{
    int i;

    type_register_static(&cris_cpu_type_info);
    for (i = 0; i < ARRAY_SIZE(cris_cpus); i++) {
        cpu_register(&cris_cpus[i]);
    }
}

type_init(cris_cpu_register_types)
