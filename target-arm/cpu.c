/*
 * QEMU ARM CPU
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

#include "cpu-qom.h"
#include "qemu-common.h"

static void arm_cpu_reset(CPUState *c)
{
    ARMCPU *cpu = ARM_CPU(c);
    ARMCPUClass *class = ARM_CPU_GET_CLASS(cpu);

    class->parent_reset(c);

    /* TODO Drop this in favor of cpu_arm_reset() calling cpu_reset()
     *      once cpu_reset_model_id() is gone. */
    cpu_state_reset(&cpu->env);
}

static void arm_cpu_class_init(ObjectClass *c, void *data)
{
    ARMCPUClass *class = ARM_CPU_CLASS(c);
    CPUClass *cpu_class = CPU_CLASS(class);

    class->parent_reset = cpu_class->reset;
    cpu_class->reset = arm_cpu_reset;
}

static const TypeInfo arm_cpu_type_info = {
    .name = TYPE_ARM_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(ARMCPU),
    .abstract = false, /* TODO Reconsider once cp15 reworked. */
    .class_size = sizeof(ARMCPUClass),
    .class_init = arm_cpu_class_init,
};

static void arm_cpu_register_types(void)
{
    type_register_static(&arm_cpu_type_info);
}

type_init(arm_cpu_register_types)
