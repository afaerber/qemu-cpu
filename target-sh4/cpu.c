/*
 * QEMU SuperH CPU
 *
 * Copyright (c) 2005 Samuel Tardieu
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

static void superh_cpu_reset(CPUState *c)
{
    SuperHCPU *cpu = SUPERH_CPU(c);
    SuperHCPUClass *klass = SUPERH_CPU_GET_CLASS(cpu);
    CPUSH4State *env = &cpu->env;

    if (qemu_loglevel_mask(CPU_LOG_RESET)) {
        qemu_log("CPU Reset (CPU %d)\n", env->cpu_index);
        log_cpu_state(env, 0);
    }

    klass->parent_reset(c);

    memset(env, 0, offsetof(CPUSH4State, breakpoints));
    tlb_flush(env, 1);

    env->pc = 0xA0000000;
#if defined(CONFIG_USER_ONLY)
    env->fpscr = FPSCR_PR; /* value for userspace according to the kernel */
    set_float_rounding_mode(float_round_nearest_even, &env->fp_status); /* ?! */
#else
    env->sr = SR_MD | SR_RB | SR_BL | SR_I3 | SR_I2 | SR_I1 | SR_I0;
    env->fpscr = FPSCR_DN | FPSCR_RM_ZERO; /* CPU reset value according to
                                              SH4 manual */
    set_float_rounding_mode(float_round_to_zero, &env->fp_status);
    set_flush_to_zero(1, &env->fp_status);
#endif
    set_default_nan_mode(1, &env->fp_status);
}

/* CPU models */

typedef struct {
    const char *name;
    int id;
    uint32_t pvr;
    uint32_t prr;
    uint32_t cvr;
    uint32_t features;
} SuperHCPUInfo;

static const SuperHCPUInfo superh_cpus[] = {
    {
        .name = "SH7750R",
        .id = SH_CPU_SH7750R,
        .pvr = 0x00050000,
        .prr = 0x00000100,
        .cvr = 0x00110000,
        .features = SH_FEATURE_BCR3_AND_BCR4,
    }, {
        .name = "SH7751R",
        .id = SH_CPU_SH7751R,
        .pvr = 0x04050005,
        .prr = 0x00000113,
        .cvr = 0x00110000,    /* Neutered caches, should be 0x20480000 */
        .features = SH_FEATURE_BCR3_AND_BCR4,
    }, {
        .name = "SH7785",
        .id = SH_CPU_SH7785,
        .pvr = 0x10300700,
        .prr = 0x00000200,
        .cvr = 0x71440211,
        .features = SH_FEATURE_SH4A,
    },
};

static void superh_cpu_initfn(Object *obj)
{
    SuperHCPU *cpu = SUPERH_CPU(obj);
    SuperHCPUClass *klass = SUPERH_CPU_GET_CLASS(cpu);
    CPUSH4State *env = &cpu->env;

    cpu->features = klass->features;

    memset(env, 0, sizeof(CPUSH4State));
    env->cpu_model_str = object_get_typename(obj);
    cpu_exec_init(env);

    env->id = klass->id;
    env->pvr = klass->pvr;
    env->prr = klass->prr;
    env->cvr = klass->cvr;

    env->movcal_backup_tail = &(env->movcal_backup);

    cpu_reset(CPU(cpu));
}

static void superh_cpu_class_init(ObjectClass *klass, void *data)
{
    CPUClass *cpu_class = CPU_CLASS(klass);
    SuperHCPUClass *k = SUPERH_CPU_CLASS(klass);
    const SuperHCPUInfo *info = data;

    k->parent_reset = cpu_class->reset;
    cpu_class->reset = superh_cpu_reset;

    k->id = info->id;
    k->pvr = info->pvr;
    k->prr = info->prr;
    k->cvr = info->cvr;
    k->features = info->features;
}

static void cpu_register(const SuperHCPUInfo *info)
{
    TypeInfo type = {
        .name = info->name,
        .parent = TYPE_SUPERH_CPU,
        .instance_size = sizeof(SuperHCPU),
        .instance_init = superh_cpu_initfn,
        .class_size = sizeof(SuperHCPUClass),
        .class_init = superh_cpu_class_init,
        .class_data = (void *)info,
    };

    type_register_static(&type);
}

static const TypeInfo superh_cpu_type_info = {
    .name = TYPE_SUPERH_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(SuperHCPU),
    .abstract = true,
    .class_size = sizeof(SuperHCPUClass),
};

static void superh_cpu_register_types(void)
{
    int i;

    type_register_static(&superh_cpu_type_info);
    for (i = 0; i < ARRAY_SIZE(superh_cpus); i++) {
        cpu_register(&superh_cpus[i]);
    }
}

type_init(superh_cpu_register_types)
