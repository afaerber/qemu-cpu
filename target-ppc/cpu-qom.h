/*
 * QEMU PowerPC CPU
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
#ifndef QEMU_PPC_CPU_QOM_H
#define QEMU_PPC_CPU_QOM_H

#include "qemu/cpu.h"
#include "cpu.h"

#define TYPE_POWERPC_CPU "powerpc-cpu"

#define POWERPC_CPU_CLASS(klass) \
    OBJECT_CLASS_CHECK(PowerPCCPUClass, (klass), TYPE_POWERPC_CPU)
#define POWERPC_CPU(obj) \
    OBJECT_CHECK(PowerPCCPU, (obj), TYPE_POWERPC_CPU)
#define POWERPC_CPU_GET_CLASS(obj) \
    OBJECT_GET_CLASS(PowerPCCPUClass, (obj), TYPE_POWERPC_CPU)

/**
 * PowerPCCPUClass:
 * @parent_reset: The parent class' reset handler.
 *
 * A PowerPC CPU model.
 */
typedef struct PowerPCCPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    void (*parent_reset)(CPUState *cpu);

    uint32_t pvr;
    uint32_t svr;
    uint64_t insns_flags;
    uint64_t insns_flags2;
    uint64_t msr_mask;
    powerpc_mmu_t   mmu_model;
    powerpc_excp_t  excp_model;
    powerpc_input_t bus_model;
    uint32_t flags;
    int bfd_mach;
    void (*init_proc)(CPUPPCState *env);
    int  (*check_pow)(CPUPPCState *env);
} PowerPCCPUClass;

/**
 * PowerPCCPU:
 * @env: Legacy CPU state.
 *
 * A PowerPC CPU.
 */
typedef struct PowerPCCPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUPPCState env;
} PowerPCCPU;

static inline PowerPCCPU *ppc_env_get_cpu(CPUPPCState *env)
{
    return POWERPC_CPU(container_of(env, PowerPCCPU, env));
}

#define ENV_GET_CPU(e) CPU(ppc_env_get_cpu(e))


#endif
