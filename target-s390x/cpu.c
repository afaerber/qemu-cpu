/*
 * QEMU S/390 CPU
 *
 * Copyright (c) 2009 Ulrich Hecht
 * Copyright (c) 2011 Alexander Graf
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
#include "qemu-timer.h"


static void s390_cpu_reset(CPUState *c)
{
    S390CPU *cpu = S390_CPU(c);
    S390CPUClass *klass = S390_CPU_GET_CLASS(c);
    CPUS390XState *env = &cpu->env;

    if (qemu_loglevel_mask(CPU_LOG_RESET)) {
        qemu_log("CPU Reset (CPU %d)\n", env->cpu_index);
        log_cpu_state(env, 0);
    }

    klass->parent_reset(c);

    memset(env, 0, offsetof(CPUS390XState, breakpoints));
    /* FIXME: reset vector? */
    tlb_flush(env, 1);
    s390_add_running_cpu(env);
}

static void s390_cpu_initfn(Object *obj)
{
    S390CPU *cpu = S390_CPU(obj);
    CPUS390XState *env = &cpu->env;
#if !defined(CONFIG_USER_ONLY)
    struct tm tm;
#endif
    static int cpu_num;

    memset(&cpu->env, 0, sizeof(CPUS390XState));
    cpu_exec_init(env);

#if !defined(CONFIG_USER_ONLY)
    qemu_get_timedate(&tm, 0);
    env->tod_offset = TOD_UNIX_EPOCH +
                      (time2tod(mktimegm(&tm)) * 1000000000ULL);
    env->tod_basetime = 0;
    env->tod_timer = qemu_new_timer_ns(vm_clock, s390x_tod_timer, cpu);
    env->cpu_timer = qemu_new_timer_ns(vm_clock, s390x_cpu_timer, cpu);
#endif
    env->cpu_num = cpu_num++;
    env->ext_index = -1;

    cpu_reset(CPU(cpu));
}

static void s390_cpu_class_init(ObjectClass *klass, void *data)
{
    S390CPUClass *k = S390_CPU_CLASS(klass);
    CPUClass *cpu_class = CPU_CLASS(klass);

    k->parent_reset = cpu_class->reset;
    cpu_class->reset = s390_cpu_reset;
}

static const TypeInfo s390_cpu_info = {
    .name = TYPE_S390_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(S390CPU),
    .instance_init = s390_cpu_initfn,
    .class_size = sizeof(S390CPUClass),
    .class_init = s390_cpu_class_init,
};

static void s390_cpu_register_types(void)
{
    type_register_static(&s390_cpu_info);
}

type_init(s390_cpu_register_types)
