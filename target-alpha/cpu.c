/*
 * QEMU Alpha CPU
 *
 * Copyright (c) 2007 Jocelyn Mayer
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

#include "cpu.h"
#include "qemu-common.h"
#include "error.h"


static void alpha_cpu_realize(Object *obj, Error **err)
{
#ifndef CONFIG_USER_ONLY
    AlphaCPU *cpu = ALPHA_CPU(obj);

    qemu_init_vcpu(&cpu->env);
#endif
}

/* Models */

#define TYPE(model) model "-" TYPE_ALPHA_CPU

typedef struct AlphaCPUAlias {
    const char *alias;
    const char *typename;
} AlphaCPUAlias;

static const AlphaCPUAlias alpha_cpu_aliases[] = {
    { "21064",   TYPE("ev4") },
    { "21164",   TYPE("ev5") },
    { "21164a",  TYPE("ev56") },
    { "21164pc", TYPE("pca56") },
    { "21264",   TYPE("ev6") },
    { "21264a",  TYPE("ev67") },
};

static ObjectClass *alpha_cpu_class_by_name(const char *cpu_model)
{
    ObjectClass *oc = NULL;
    char *typename;
    int i;

    if (cpu_model == NULL) {
        return NULL;
    }

    oc = object_class_by_name(cpu_model);
    if (oc != NULL) {
        return oc;
    }

    for (i = 0; i < ARRAY_SIZE(alpha_cpu_aliases); i++) {
        if (strcmp(cpu_model, alpha_cpu_aliases[i].alias) == 0) {
            oc = object_class_by_name(alpha_cpu_aliases[i].typename);
            assert(oc != NULL);
            return oc;
        }
    }

    typename = g_strdup_printf("%s-" TYPE_ALPHA_CPU, cpu_model);
    oc = object_class_by_name(typename);
    g_free(typename);
    return oc;
}

AlphaCPU *cpu_alpha_init(const char *cpu_model)
{
    AlphaCPU *cpu;
    CPUAlphaState *env;
    ObjectClass *cpu_class;

    cpu_class = alpha_cpu_class_by_name(cpu_model);
    if (cpu_class == NULL) {
        /* Default to ev67; no reason not to emulate insns by default.  */
        cpu_class = object_class_by_name(TYPE("ev67"));
    }
    cpu = ALPHA_CPU(object_new(object_class_get_name(cpu_class)));
    env = &cpu->env;

    env->cpu_model_str = cpu_model;

    alpha_cpu_realize(OBJECT(cpu), NULL);
    return cpu;
}

static void ev4_cpu_initfn(Object *obj)
{
    AlphaCPU *cpu = ALPHA_CPU(obj);
    CPUAlphaState *env = &cpu->env;

    env->implver = IMPLVER_2106x;
}

static const TypeInfo ev4_cpu_type_info = {
    .name = TYPE("ev4"),
    .parent = TYPE_ALPHA_CPU,
    .instance_init = ev4_cpu_initfn,
};

static void ev5_cpu_initfn(Object *obj)
{
    AlphaCPU *cpu = ALPHA_CPU(obj);
    CPUAlphaState *env = &cpu->env;

    env->implver = IMPLVER_21164;
}

static const TypeInfo ev5_cpu_type_info = {
    .name = TYPE("ev5"),
    .parent = TYPE_ALPHA_CPU,
    .instance_init = ev5_cpu_initfn,
};

static void ev56_cpu_initfn(Object *obj)
{
    AlphaCPU *cpu = ALPHA_CPU(obj);
    CPUAlphaState *env = &cpu->env;

    env->amask |= AMASK_BWX;
}

static const TypeInfo ev56_cpu_type_info = {
    .name = TYPE("ev56"),
    .parent = TYPE("ev5"),
    .instance_init = ev56_cpu_initfn,
};

static void pca56_cpu_initfn(Object *obj)
{
    AlphaCPU *cpu = ALPHA_CPU(obj);
    CPUAlphaState *env = &cpu->env;

    env->amask |= AMASK_MVI;
}

static const TypeInfo pca56_cpu_type_info = {
    .name = TYPE("pca56"),
    .parent = TYPE("ev56"),
    .instance_init = pca56_cpu_initfn,
};

static void ev6_cpu_initfn(Object *obj)
{
    AlphaCPU *cpu = ALPHA_CPU(obj);
    CPUAlphaState *env = &cpu->env;

    env->implver = IMPLVER_21264;
    env->amask = AMASK_BWX | AMASK_FIX | AMASK_MVI | AMASK_TRAP;
}

static const TypeInfo ev6_cpu_type_info = {
    .name = TYPE("ev6"),
    .parent = TYPE_ALPHA_CPU,
    .instance_init = ev6_cpu_initfn,
};

static void ev67_cpu_initfn(Object *obj)
{
    AlphaCPU *cpu = ALPHA_CPU(obj);
    CPUAlphaState *env = &cpu->env;

    env->amask |= AMASK_CIX | AMASK_PREFETCH;
}

static const TypeInfo ev67_cpu_type_info = {
    .name = TYPE("ev67"),
    .parent = TYPE("ev6"),
    .instance_init = ev67_cpu_initfn,
};

static const TypeInfo ev68_cpu_type_info = {
    .name = TYPE("ev68"),
    .parent = TYPE("ev67"),
};

static void alpha_cpu_initfn(Object *obj)
{
    AlphaCPU *cpu = ALPHA_CPU(obj);
    CPUAlphaState *env = &cpu->env;

    cpu_exec_init(env);
    tlb_flush(env, 1);

    alpha_translate_init();

#if defined(CONFIG_USER_ONLY)
    env->ps = PS_USER_MODE;
    cpu_alpha_store_fpcr(env, (FPCR_INVD | FPCR_DZED | FPCR_OVFD
                               | FPCR_UNFD | FPCR_INED | FPCR_DNOD
                               | FPCR_DYN_NORMAL));
#endif
    env->lock_addr = -1;
    env->fen = 1;
}

static const TypeInfo alpha_cpu_type_info = {
    .name = TYPE_ALPHA_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(AlphaCPU),
    .instance_init = alpha_cpu_initfn,
    .abstract = true,
    .class_size = sizeof(AlphaCPUClass),
};

static void alpha_cpu_register_types(void)
{
    type_register_static(&alpha_cpu_type_info);
    type_register_static(&ev4_cpu_type_info);
    type_register_static(&ev5_cpu_type_info);
    type_register_static(&ev56_cpu_type_info);
    type_register_static(&pca56_cpu_type_info);
    type_register_static(&ev6_cpu_type_info);
    type_register_static(&ev67_cpu_type_info);
    type_register_static(&ev68_cpu_type_info);
}

type_init(alpha_cpu_register_types)
