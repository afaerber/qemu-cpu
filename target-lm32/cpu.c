/*
 * QEMU LatticeMico32 CPU
 *
 * Copyright (c) 2010 Michael Walle <michael@walle.cc>
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

static void lm32_cpu_reset(CPUState *c)
{
}

/* CPU models */

typedef struct LM32CPUInfo {
    const char *name;
    uint32_t revision;
    uint8_t num_interrupts;
    uint8_t num_breakpoints;
    uint8_t num_watchpoints;
    uint32_t features;
} LM32CPUInfo;

static const LM32CPUInfo lm32_cpus[] = {
    {
        .name = "lm32-basic",
        .revision = 3,
        .num_interrupts = 32,
        .num_breakpoints = 4,
        .num_watchpoints = 4,
        .features = (LM32_FEATURE_SHIFT
                     | LM32_FEATURE_SIGN_EXTEND
                     | LM32_FEATURE_CYCLE_COUNT),
    },
    {
        .name = "lm32-standard",
        .revision = 3,
        .num_interrupts = 32,
        .num_breakpoints = 4,
        .num_watchpoints = 4,
        .features = (LM32_FEATURE_MULTIPLY
                     | LM32_FEATURE_DIVIDE
                     | LM32_FEATURE_SHIFT
                     | LM32_FEATURE_SIGN_EXTEND
                     | LM32_FEATURE_I_CACHE
                     | LM32_FEATURE_CYCLE_COUNT),
    },
    {
        .name = "lm32-full",
        .revision = 3,
        .num_interrupts = 32,
        .num_breakpoints = 4,
        .num_watchpoints = 4,
        .features = (LM32_FEATURE_MULTIPLY
                     | LM32_FEATURE_DIVIDE
                     | LM32_FEATURE_SHIFT
                     | LM32_FEATURE_SIGN_EXTEND
                     | LM32_FEATURE_I_CACHE
                     | LM32_FEATURE_D_CACHE
                     | LM32_FEATURE_CYCLE_COUNT),
    }
};

static uint32_t cfg_by_class(LM32CPUClass *def)
{
    uint32_t cfg = 0;

    if (def->features & LM32_FEATURE_MULTIPLY) {
        cfg |= CFG_M;
    }

    if (def->features & LM32_FEATURE_DIVIDE) {
        cfg |= CFG_D;
    }

    if (def->features & LM32_FEATURE_SHIFT) {
        cfg |= CFG_S;
    }

    if (def->features & LM32_FEATURE_SIGN_EXTEND) {
        cfg |= CFG_X;
    }

    if (def->features & LM32_FEATURE_I_CACHE) {
        cfg |= CFG_IC;
    }

    if (def->features & LM32_FEATURE_D_CACHE) {
        cfg |= CFG_DC;
    }

    if (def->features & LM32_FEATURE_CYCLE_COUNT) {
        cfg |= CFG_CC;
    }

    cfg |= (def->num_interrupts << CFG_INT_SHIFT);
    cfg |= (def->num_breakpoints << CFG_BP_SHIFT);
    cfg |= (def->num_watchpoints << CFG_WP_SHIFT);
    cfg |= (def->revision << CFG_REV_SHIFT);

    return cfg;
}

static void lm32_cpu_initfn(Object *obj)
{
    LM32CPU *cpu = LM32_CPU(obj);
    LM32CPUClass *klass = LM32_CPU_GET_CLASS(cpu);
    CPULM32State *env = &cpu->env;

    memset(env, 0, sizeof(*env));
    cpu_exec_init(env);
    env->cpu_model_str = object_get_typename(obj);

    env->features = klass->features;
    env->num_bps = klass->num_breakpoints;
    env->num_wps = klass->num_watchpoints;
    env->cfg = cfg_by_class(klass);
    env->flags = 0;

    cpu_reset(CPU(cpu));
}

static void lm32_cpu_class_init(ObjectClass *klass, void *data)
{
    CPUClass *cpu_class = CPU_CLASS(klass);
    LM32CPUClass *k = LM32_CPU_CLASS(klass);
    const LM32CPUInfo *info = data;

    k->parent_reset = cpu_class->reset;
    cpu_class->reset = lm32_cpu_reset;

    k->revision = info->revision;
    k->num_interrupts = info->num_interrupts;
    k->num_breakpoints = info->num_breakpoints;
    k->num_watchpoints = info->num_watchpoints;
    k->features = info->features;
}

static void cpu_register(const LM32CPUInfo *info)
{
    TypeInfo type = {
        .name = info->name,
        .parent = TYPE_LM32_CPU,
        .instance_size = sizeof(LM32CPU),
        .instance_init = lm32_cpu_initfn,
        .class_size = sizeof(LM32CPUClass),
        .class_init = lm32_cpu_class_init,
        .class_data = (void *)info,
    };

    type_register_static(&type);
}

static const TypeInfo lm32_cpu_type_info = {
    .name = TYPE_LM32_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(LM32CPU),
    .abstract = true,
    .class_size = sizeof(LM32CPUClass),
};

static void lm32_cpu_register_types(void)
{
    int i;

    type_register_static(&lm32_cpu_type_info);
    for (i = 0; i < ARRAY_SIZE(lm32_cpus); i++) {
        cpu_register(&lm32_cpus[i]);
    }
}

type_init(lm32_cpu_register_types)
