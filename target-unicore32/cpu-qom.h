/*
 * QEMU UniCore32 CPU
 *
 * Copyright (c) 2012 SUSE LINUX Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>
 */
#ifndef QEMU_UC32_CPU_QOM_H
#define QEMU_UC32_CPU_QOM_H

#include "qemu/cpu.h"
#include "cpu.h"

#define TYPE_UNICORE32_CPU "unicore32-cpu"

#define UNICORE32_CPU_CLASS(klass) \
    OBJECT_CLASS_CHECK(UniCore32CPUClass, (klass), TYPE_UNICORE32_CPU)
#define UNICORE32_CPU(obj) \
    OBJECT_CHECK(UniCore32CPU, (obj), TYPE_UNICORE32_CPU)
#define UNICORE32_CPU_GET_CLASS(obj) \
    OBJECT_GET_CLASS(UniCore32CPUClass, (obj), TYPE_UNICORE32_CPU)

/**
 * UniCore32CPUClass:
 *
 * A UniCore32 CPU model.
 */
typedef struct UniCore32CPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    struct {
        uint32_t c0_cpuid;
        uint32_t c0_cachetype;
        uint32_t c1_sys;
    } cp0;
} UniCore32CPUClass;

/**
 * UniCore32CPU:
 * @env: Legacy CPU state.
 *
 * A UniCore32 CPU.
 */
typedef struct UniCore32CPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUUniCore32State env;
} UniCore32CPU;

static inline UniCore32CPU *uc32_env_get_cpu(CPUUniCore32State *env)
{
    return UNICORE32_CPU(container_of(env, UniCore32CPU, env));
}

#define ENV_GET_CPU(e) CPU(uc32_env_get_cpu(e))


#endif
