/*
 * QEMU Alpha CPU
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

static void alpha_cpu_reset(CPUState *c)
{
    AlphaCPU *cpu = ALPHA_CPU(c);
    AlphaCPUClass *klass = ALPHA_CPU_GET_CLASS(cpu);

    klass->parent_reset(c);
}

/* CPU models */

typedef struct AlphaCPUInfo {
    const char *name;
    int implver, amask;
} AlphaCPUInfo;

static const AlphaCPUInfo alpha_cpus[] = {
    { "ev4",   IMPLVER_2106x, 0 },
    { "ev5",   IMPLVER_21164, 0 },
    { "ev56",  IMPLVER_21164, AMASK_BWX },
    { "pca56", IMPLVER_21164, AMASK_BWX | AMASK_MVI },
    { "ev6",   IMPLVER_21264, AMASK_BWX | AMASK_FIX | AMASK_MVI | AMASK_TRAP },
    { "ev67",  IMPLVER_21264, (AMASK_BWX | AMASK_FIX | AMASK_CIX
                               | AMASK_MVI | AMASK_TRAP | AMASK_PREFETCH), },
    { "ev68",  IMPLVER_21264, (AMASK_BWX | AMASK_FIX | AMASK_CIX
                               | AMASK_MVI | AMASK_TRAP | AMASK_PREFETCH), },
    { "21064", IMPLVER_2106x, 0 },
    { "21164", IMPLVER_21164, 0 },
    { "21164a", IMPLVER_21164, AMASK_BWX },
    { "21164pc", IMPLVER_21164, AMASK_BWX | AMASK_MVI },
    { "21264", IMPLVER_21264, AMASK_BWX | AMASK_FIX | AMASK_MVI | AMASK_TRAP },
    { "21264a", IMPLVER_21264, (AMASK_BWX | AMASK_FIX | AMASK_CIX
                                | AMASK_MVI | AMASK_TRAP | AMASK_PREFETCH), }
};

static void alpha_cpu_initfn(Object *obj)
{
    AlphaCPU *cpu = ALPHA_CPU(obj);
    AlphaCPUClass *klass = ALPHA_CPU_GET_CLASS(cpu);
    CPUAlphaState *env = &cpu->env;

    env->implver = klass->implver;
    env->amask = klass->amask;

    memset(env, 0, sizeof(CPUAlphaState));
    cpu_exec_init(env);
    tlb_flush(env, 1);

#if defined(CONFIG_USER_ONLY)
    env->ps = PS_USER_MODE;
    cpu_alpha_store_fpcr(env, (FPCR_INVD | FPCR_DZED | FPCR_OVFD
                               | FPCR_UNFD | FPCR_INED | FPCR_DNOD
                               | FPCR_DYN_NORMAL));
#endif
    env->lock_addr = -1;
    env->fen = 1;
}

static void alpha_cpu_class_init(ObjectClass *klass, void *data)
{
    AlphaCPUClass *k = ALPHA_CPU_CLASS(klass);
    CPUClass *cpu_class = CPU_CLASS(klass);
    const AlphaCPUInfo *info = data;

    k->parent_reset = cpu_class->reset;
    cpu_class->reset = alpha_cpu_reset;

    k->implver = info->implver;
    k->amask = info->amask;
}

static void alpha_register_cpu(const AlphaCPUInfo *info)
{
    TypeInfo type = {
        .name = info->name,
        .parent = TYPE_ALPHA_CPU,
        .instance_size = sizeof(AlphaCPU),
        .instance_init = alpha_cpu_initfn,
        .class_size = sizeof(AlphaCPUClass),
        .class_init = alpha_cpu_class_init,
        .class_data = (void *)info,
    };

    type_register_static(&type);
}

static const TypeInfo alpha_cpu_info = {
    .name = TYPE_ALPHA_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(AlphaCPU),
    .instance_init = alpha_cpu_initfn,
    .abstract = true,
    .class_size = sizeof(AlphaCPUClass),
};

static void alpha_cpu_register_types(void)
{
    int i;

    type_register_static(&alpha_cpu_info);
    for (i = 0; i < ARRAY_SIZE(alpha_cpus); i++) {
        alpha_register_cpu(&alpha_cpus[i]);
    }
}

type_init(alpha_cpu_register_types)
