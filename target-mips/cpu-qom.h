/*
 * QEMU MIPS CPU
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
#ifndef QEMU_MIPS_CPU_QOM_H
#define QEMU_MIPS_CPU_QOM_H

#include "qemu-common.h"
#include "qemu/cpu.h"
#include "cpu.h"

#define TYPE_MIPS_CPU "mips-cpu"

/**
 * MIPSMMUTypes:
 *
 * MMU types, the first four entries have the same layout as the
 * CP0C0_MT field.
 */
enum MIPSMMUTypes {
    MMU_TYPE_NONE,
    MMU_TYPE_R4000,
    MMU_TYPE_RESERVED,
    MMU_TYPE_FMT,
    MMU_TYPE_R3000,
    MMU_TYPE_R6000,
    MMU_TYPE_R8000
};

#define MIPS_CPU_CLASS(klass) \
    OBJECT_CLASS_CHECK(MIPSCPUClass, (klass), TYPE_MIPS_CPU)
#define MIPS_CPU(obj) \
    OBJECT_CHECK(MIPSCPU, (obj), TYPE_MIPS_CPU)
#define MIPS_CPU_GET_CLASS(obj) \
    OBJECT_GET_CLASS(MIPSCPUClass, (obj), TYPE_MIPS_CPU)

/**
 * MIPSCPUClass:
 * @parent_reset: The parent class' reset handler.
 *
 * A MIPS CPU model.
 */
typedef struct MIPSCPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    void (*parent_reset)(CPUState *cpu);

    int32_t cp0_prid;
    int32_t cp0_config0;
    int32_t cp0_config1;
    int32_t cp0_config2;
    int32_t cp0_config3;
    int32_t cp0_config6;
    int32_t cp0_config7;
    target_ulong cp0_lladdr_rw_bitmask;
    int cp0_lladdr_shift;
    int32_t synci_step;
    int32_t ccres;
    int32_t cp0_status_rw_bitmask;
    int32_t cp0_tcstatus_rw_bitmask;
    int32_t cp0_srsctl;
    int32_t cp1_fcr0;
    int32_t segbits;
    int32_t pabits;
    int32_t cp0_srsconf_rw_bitmask[5];
    int32_t cp0_srsconf[5];
    int insn_flags;
    enum MIPSMMUTypes mmu_type;
} MIPSCPUClass;

/**
 * MIPSCPU:
 * @env: Legacy CPU state.
 *
 * A MIPS CPU.
 */
typedef struct MIPSCPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUMIPSState env;
} MIPSCPU;

static inline MIPSCPU *mips_env_get_cpu(CPUMIPSState *env)
{
    return MIPS_CPU(container_of(env, MIPSCPU, env));
}

#define ENV_GET_CPU(e) CPU(mips_env_get_cpu(e))

#define ENV_OFFSET offsetof(MIPSCPU, env)


#endif
