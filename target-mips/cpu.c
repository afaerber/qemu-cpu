/*
 * QEMU MIPS CPU
 *
 * Copyright (c) 2004-2005 Jocelyn Mayer
 * Copyright (c) 2007 Herve Poussineau
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

#ifndef CONFIG_USER_ONLY
static void no_mmu_init(MIPSCPU *cpu)
{
    CPUMIPSState *env = &cpu->env;

    env->tlb->nb_tlb = 1;
    env->tlb->map_address = &no_mmu_map_address;
}

static void fixed_mmu_init(MIPSCPU *cpu)
{
    CPUMIPSState *env = &cpu->env;

    env->tlb->nb_tlb = 1;
    env->tlb->map_address = &fixed_mmu_map_address;
}

static void r4k_mmu_init(MIPSCPU *cpu)
{
    MIPSCPUClass *cpu_class = MIPS_CPU_GET_CLASS(cpu);
    CPUMIPSState *env = &cpu->env;

    env->tlb->nb_tlb = 1 + ((cpu_class->cp0_config1 >> CP0C1_MMU) & 63);
    env->tlb->map_address = &r4k_map_address;
    env->tlb->helper_tlbwi = r4k_helper_tlbwi;
    env->tlb->helper_tlbwr = r4k_helper_tlbwr;
    env->tlb->helper_tlbp = r4k_helper_tlbp;
    env->tlb->helper_tlbr = r4k_helper_tlbr;
}

static void mmu_init(MIPSCPU *cpu)
{
    MIPSCPUClass *cpu_class = MIPS_CPU_GET_CLASS(cpu);
    CPUMIPSState *env = &cpu->env;

    env->tlb = g_malloc0(sizeof(CPUMIPSTLBContext));

    switch (cpu_class->mmu_type) {
    case MMU_TYPE_NONE:
        no_mmu_init(cpu);
        break;
    case MMU_TYPE_R4000:
        r4k_mmu_init(cpu);
        break;
    case MMU_TYPE_FMT:
        fixed_mmu_init(cpu);
        break;
    case MMU_TYPE_R3000:
    case MMU_TYPE_R6000:
    case MMU_TYPE_R8000:
    default:
        cpu_abort(env, "MMU type not supported\n");
    }
}
#endif /* CONFIG_USER_ONLY */

static void fpu_init(MIPSCPU *cpu)
{
    MIPSCPUClass *cpu_class = MIPS_CPU_GET_CLASS(cpu);
    CPUMIPSState *env = &cpu->env;
    int i;

    for (i = 0; i < MIPS_FPU_MAX; i++) {
        env->fpus[i].fcr0 = cpu_class->cp1_fcr0;
    }

    memcpy(&env->active_fpu, &env->fpus[0], sizeof(env->active_fpu));
}

static void mvp_init(MIPSCPU *cpu)
{
    CPUMIPSState *env = &cpu->env;
    env->mvp = g_malloc0(sizeof(CPUMIPSMVPContext));

    /* MVPConf1 implemented, TLB sharable, no gating storage support,
       programmable cache partitioning implemented, number of allocatable
       and sharable TLB entries, MVP has allocatable TCs, 2 VPEs
       implemented, 5 TCs implemented. */
    env->mvp->CP0_MVPConf0 = (1 << CP0MVPC0_M) | (1 << CP0MVPC0_TLBS) |
                             (0 << CP0MVPC0_GS) | (1 << CP0MVPC0_PCP) |
/* TODO: actually do 2 VPEs.
 *                           (1 << CP0MVPC0_TCA) | (0x1 << CP0MVPC0_PVPE) |
 *                           (0x04 << CP0MVPC0_PTC);
 */
                             (1 << CP0MVPC0_TCA) | (0x0 << CP0MVPC0_PVPE) |
                             (0x00 << CP0MVPC0_PTC);
#if !defined(CONFIG_USER_ONLY)
    /* Usermode has no TLB support */
    env->mvp->CP0_MVPConf0 |= (env->tlb->nb_tlb << CP0MVPC0_PTLBE);
#endif

    /* Allocatable CP1 have media extensions, allocatable CP1 have FP support,
       no UDI implemented, no CP2 implemented, 1 CP1 implemented. */
    env->mvp->CP0_MVPConf1 = (1 << CP0MVPC1_CIM) | (1 << CP0MVPC1_CIF) |
                             (0x0 << CP0MVPC1_PCX) | (0x0 << CP0MVPC1_PCP2) |
                             (0x1 << CP0MVPC1_PCP1);
}

static void mips_cpu_reset(CPUState *c)
{
    MIPSCPU *cpu = MIPS_CPU(c);
    MIPSCPUClass *cpu_class = MIPS_CPU_GET_CLASS(cpu);
    CPUMIPSState *env = &cpu->env;

    if (qemu_loglevel_mask(CPU_LOG_RESET)) {
        qemu_log("CPU Reset (CPU %d)\n", env->cpu_index);
        log_cpu_state(env, 0);
    }

    cpu_class->parent_reset(c);

    memset(env, 0, offsetof(CPUMIPSState, breakpoints));
    tlb_flush(env, 1);

    /* Reset registers to their default values */
    env->CP0_PRid = cpu_class->cp0_prid;
    env->CP0_Config0 = cpu_class->cp0_config0;
#ifdef TARGET_WORDS_BIGENDIAN
    env->CP0_Config0 |= (1 << CP0C0_BE);
#endif
    env->CP0_Config1 = cpu_class->cp0_config1;
    env->CP0_Config2 = cpu_class->cp0_config2;
    env->CP0_Config3 = cpu_class->cp0_config3;
    env->CP0_Config6 = cpu_class->cp0_config6;
    env->CP0_Config7 = cpu_class->cp0_config7;
    env->CP0_LLAddr_rw_bitmask = cpu_class->cp0_lladdr_rw_bitmask
                                 << cpu_class->cp0_lladdr_shift;
    env->CP0_LLAddr_shift = cpu_class->cp0_lladdr_shift;
    env->SYNCI_Step = cpu_class->synci_step;
    env->CCRes = cpu_class->ccres;
    env->CP0_Status_rw_bitmask = cpu_class->cp0_status_rw_bitmask;
    env->CP0_TCStatus_rw_bitmask = cpu_class->cp0_tcstatus_rw_bitmask;
    env->CP0_SRSCtl = cpu_class->cp0_srsctl;
    env->current_tc = 0;
    env->SEGBITS = cpu_class->segbits;
    env->SEGMask = (target_ulong)((1ULL << cpu_class->segbits) - 1);
#if defined(TARGET_MIPS64)
    if (cpu_class->insn_flags & ISA_MIPS3) {
        env->SEGMask |= 3ULL << 62;
    }
#endif
    env->PABITS = cpu_class->pabits;
    env->PAMask = (target_ulong)((1ULL << cpu_class->pabits) - 1);
    env->CP0_SRSConf0_rw_bitmask = cpu_class->cp0_srsconf_rw_bitmask[0];
    env->CP0_SRSConf0 = cpu_class->cp0_srsconf[0];
    env->CP0_SRSConf1_rw_bitmask = cpu_class->cp0_srsconf_rw_bitmask[1];
    env->CP0_SRSConf1 = cpu_class->cp0_srsconf[1];
    env->CP0_SRSConf2_rw_bitmask = cpu_class->cp0_srsconf_rw_bitmask[2];
    env->CP0_SRSConf2 = cpu_class->cp0_srsconf[2];
    env->CP0_SRSConf3_rw_bitmask = cpu_class->cp0_srsconf_rw_bitmask[3];
    env->CP0_SRSConf3 = cpu_class->cp0_srsconf[3];
    env->CP0_SRSConf4_rw_bitmask = cpu_class->cp0_srsconf_rw_bitmask[4];
    env->CP0_SRSConf4 = cpu_class->cp0_srsconf[4];
    env->insn_flags = cpu_class->insn_flags;

#if defined(CONFIG_USER_ONLY)
    env->hflags = MIPS_HFLAG_UM;
    /* Enable access to the SYNCI_Step register.  */
    env->CP0_HWREna |= (1 << 1);
    if (env->CP0_Config1 & (1 << CP0C1_FP)) {
        env->hflags |= MIPS_HFLAG_FPU;
    }
#ifdef TARGET_MIPS64
    if (env->active_fpu.fcr0 & (1 << FCR0_F64)) {
        env->hflags |= MIPS_HFLAG_F64;
    }
#endif
#else
    if (env->hflags & MIPS_HFLAG_BMASK) {
        /* If the exception was raised from a delay slot,
           come back to the jump.  */
        env->CP0_ErrorEPC = env->active_tc.PC - 4;
    } else {
        env->CP0_ErrorEPC = env->active_tc.PC;
    }
    env->active_tc.PC = (int32_t)0xBFC00000;
    env->CP0_Random = env->tlb->nb_tlb - 1;
    env->tlb->tlb_in_use = env->tlb->nb_tlb;
    env->CP0_Wired = 0;
    env->CP0_EBase = 0x80000000 | (env->cpu_index & 0x3FF);
    env->CP0_Status = (1 << CP0St_BEV) | (1 << CP0St_ERL);
    /* vectored interrupts not implemented, timer on int 7,
       no performance counters. */
    env->CP0_IntCtl = 0xe0000000;
    {
        int i;

        for (i = 0; i < 7; i++) {
            env->CP0_WatchLo[i] = 0;
            env->CP0_WatchHi[i] = 0x80000000;
        }
        env->CP0_WatchLo[7] = 0;
        env->CP0_WatchHi[7] = 0;
    }
    /* Count register increments in debug mode, EJTAG version 1 */
    env->CP0_Debug = (1 << CP0DB_CNT) | (0x1 << CP0DB_VER);
    env->hflags = MIPS_HFLAG_CP0;

    if (env->CP0_Config3 & (1 << CP0C3_MT)) {
        int i;

        /* Only TC0 on VPE 0 starts as active.  */
        for (i = 0; i < ARRAY_SIZE(env->tcs); i++) {
            env->tcs[i].CP0_TCBind = env->cpu_index << CP0TCBd_CurVPE;
            env->tcs[i].CP0_TCHalt = 1;
        }
        env->active_tc.CP0_TCHalt = 1;
        env->halted = 1;

        if (!env->cpu_index) {
            /* VPE0 starts up enabled.  */
            env->mvp->CP0_MVPControl |= (1 << CP0MVPCo_EVP);
            env->CP0_VPEConf0 |= (1 << CP0VPEC0_MVP) | (1 << CP0VPEC0_VPA);

            /* TC0 starts up unhalted.  */
            env->halted = 0;
            env->active_tc.CP0_TCHalt = 0;
            env->tcs[0].CP0_TCHalt = 0;
            /* With thread 0 active.  */
            env->active_tc.CP0_TCStatus = (1 << CP0TCSt_A);
            env->tcs[0].CP0_TCStatus = (1 << CP0TCSt_A);
        }
    }
#endif
#if defined(TARGET_MIPS64)
    if (cpu_class->insn_flags & ISA_MIPS3) {
        env->hflags |= MIPS_HFLAG_64;
    }
#endif
    env->exception_index = EXCP_NONE;
}

/* CPU / CPU family specific config register values. */

/* Have config1, uncached coherency */
#define MIPS_CONFIG0                                              \
  ((1 << CP0C0_M) | (0x2 << CP0C0_K0))

/* Have config2, no coprocessor2 attached, no MDMX support attached,
   no performance counters, watch registers present,
   no code compression, EJTAG present, no FPU */
#define MIPS_CONFIG1                                              \
((1 << CP0C1_M) |                                                 \
 (0 << CP0C1_C2) | (0 << CP0C1_MD) | (0 << CP0C1_PC) |            \
 (1 << CP0C1_WR) | (0 << CP0C1_CA) | (1 << CP0C1_EP) |            \
 (0 << CP0C1_FP))

/* Have config3, no tertiary/secondary caches implemented */
#define MIPS_CONFIG2                                              \
((1 << CP0C2_M))

/* No config4, no DSP ASE, no large physaddr (PABITS),
   no external interrupt controller, no vectored interrupts,
   no 1kb pages, no SmartMIPS ASE, no trace logic */
#define MIPS_CONFIG3                                              \
((0 << CP0C3_M) | (0 << CP0C3_DSPP) | (0 << CP0C3_LPA) |          \
 (0 << CP0C3_VEIC) | (0 << CP0C3_VInt) | (0 << CP0C3_SP) |        \
 (0 << CP0C3_SM) | (0 << CP0C3_TL))

typedef struct MIPSCPUInfo {
    const char *name;
    int32_t CP0_PRid;
    int32_t CP0_Config0;
    int32_t CP0_Config1;
    int32_t CP0_Config2;
    int32_t CP0_Config3;
    int32_t CP0_Config6;
    int32_t CP0_Config7;
    target_ulong CP0_LLAddr_rw_bitmask;
    int CP0_LLAddr_shift;
    int32_t SYNCI_Step;
    int32_t CCRes;
    int32_t CP0_Status_rw_bitmask;
    int32_t CP0_TCStatus_rw_bitmask;
    int32_t CP0_SRSCtl;
    int32_t CP1_fcr0;
    int32_t SEGBITS;
    int32_t PABITS;
    int32_t CP0_SRSConf0_rw_bitmask;
    int32_t CP0_SRSConf0;
    int32_t CP0_SRSConf1_rw_bitmask;
    int32_t CP0_SRSConf1;
    int32_t CP0_SRSConf2_rw_bitmask;
    int32_t CP0_SRSConf2;
    int32_t CP0_SRSConf3_rw_bitmask;
    int32_t CP0_SRSConf3;
    int32_t CP0_SRSConf4_rw_bitmask;
    int32_t CP0_SRSConf4;
    int insn_flags;
    enum MIPSMMUTypes mmu_type;
} MIPSCPUInfo;

/*****************************************************************************/
/* MIPS CPU definitions */
static const MIPSCPUInfo mips_cpus[] = {
    {
        .name = "4Kc",
        .CP0_PRid = 0x00018000,
        .CP0_Config0 = MIPS_CONFIG0 | (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (0 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1278FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "4Km",
        .CP0_PRid = 0x00018300,
        /* Config1 implemented, fixed mapping MMU,
           no virtual icache, uncached coherency. */
        .CP0_Config0 = MIPS_CONFIG0 | (MMU_TYPE_FMT << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1258FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_FMT,
    },
    {
        .name = "4KEcR1",
        .CP0_PRid = 0x00018400,
        .CP0_Config0 = MIPS_CONFIG0 | (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (0 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1278FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "4KEmR1",
        .CP0_PRid = 0x00018500,
        .CP0_Config0 = MIPS_CONFIG0 | (MMU_TYPE_FMT << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1258FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_FMT,
    },
    {
        .name = "4KEc",
        .CP0_PRid = 0x00019000,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                    (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (0 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (0 << CP0C3_VInt),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1278FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "4KEm",
        .CP0_PRid = 0x00019100,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                       (MMU_TYPE_FMT << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1258FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_FMT,
    },
    {
        .name = "24Kc",
        .CP0_PRid = 0x00019300,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (0 << CP0C3_VInt),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        /* No DSP implemented. */
        .CP0_Status_rw_bitmask = 0x1278FF1F,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "24Kf",
        .CP0_PRid = 0x00019300,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                    (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (0 << CP0C3_VInt),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        /* No DSP implemented. */
        .CP0_Status_rw_bitmask = 0x3678FF1F,
        .CP1_fcr0 = (1 << FCR0_F64) | (1 << FCR0_L) | (1 << FCR0_W) |
                    (1 << FCR0_D) | (1 << FCR0_S) | (0x93 << FCR0_PRID),
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "34Kf",
        .CP0_PRid = 0x00019500,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (1 << CP0C3_VInt) | (1 << CP0C3_MT),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 0,
        .SYNCI_Step = 32,
        .CCRes = 2,
        /* No DSP implemented. */
        .CP0_Status_rw_bitmask = 0x3678FF1F,
        /* No DSP implemented. */
        .CP0_TCStatus_rw_bitmask = (0 << CP0TCSt_TCU3) | (0 << CP0TCSt_TCU2) |
                    (1 << CP0TCSt_TCU1) | (1 << CP0TCSt_TCU0) |
                    (0 << CP0TCSt_TMX) | (1 << CP0TCSt_DT) |
                    (1 << CP0TCSt_DA) | (1 << CP0TCSt_A) |
                    (0x3 << CP0TCSt_TKSU) | (1 << CP0TCSt_IXMT) |
                    (0xff << CP0TCSt_TASID),
        .CP1_fcr0 = (1 << FCR0_F64) | (1 << FCR0_L) | (1 << FCR0_W) |
                    (1 << FCR0_D) | (1 << FCR0_S) | (0x95 << FCR0_PRID),
        .CP0_SRSCtl = (0xf << CP0SRSCtl_HSS),
        .CP0_SRSConf0_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf0 = (1 << CP0SRSC0_M) | (0x3fe << CP0SRSC0_SRS3) |
                    (0x3fe << CP0SRSC0_SRS2) | (0x3fe << CP0SRSC0_SRS1),
        .CP0_SRSConf1_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf1 = (1 << CP0SRSC1_M) | (0x3fe << CP0SRSC1_SRS6) |
                    (0x3fe << CP0SRSC1_SRS5) | (0x3fe << CP0SRSC1_SRS4),
        .CP0_SRSConf2_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf2 = (1 << CP0SRSC2_M) | (0x3fe << CP0SRSC2_SRS9) |
                    (0x3fe << CP0SRSC2_SRS8) | (0x3fe << CP0SRSC2_SRS7),
        .CP0_SRSConf3_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf3 = (1 << CP0SRSC3_M) | (0x3fe << CP0SRSC3_SRS12) |
                    (0x3fe << CP0SRSC3_SRS11) | (0x3fe << CP0SRSC3_SRS10),
        .CP0_SRSConf4_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf4 = (0x3fe << CP0SRSC4_SRS15) |
                    (0x3fe << CP0SRSC4_SRS14) | (0x3fe << CP0SRSC4_SRS13),
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16 | ASE_DSP | ASE_MT,
        .mmu_type = MMU_TYPE_R4000,
    },
#if defined(TARGET_MIPS64)
    {
        .name = "R4000",
        .CP0_PRid = 0x00000400,
        /* No L2 cache, icache size 8k, dcache size 8k, uncached coherency. */
        .CP0_Config0 = (1 << 17) | (0x1 << 9) | (0x1 << 6) | (0x2 << CP0C0_K0),
        /* Note: Config1 is only used internally, the R4000 has only Config0. */
        .CP0_Config1 = (1 << CP0C1_FP) | (47 << CP0C1_MMU),
        .CP0_LLAddr_rw_bitmask = 0xFFFFFFFF,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 16,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x3678FFFF,
        /* The R4000 has a full 64bit FPU but doesn't use the fcr0 bits. */
        .CP1_fcr0 = (0x5 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 40,
        .PABITS = 36,
        .insn_flags = CPU_MIPS3,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "VR5432",
        .CP0_PRid = 0x00005400,
        /* No L2 cache, icache size 8k, dcache size 8k, uncached coherency. */
        .CP0_Config0 = (1 << 17) | (0x1 << 9) | (0x1 << 6) | (0x2 << CP0C0_K0),
        .CP0_Config1 = (1 << CP0C1_FP) | (47 << CP0C1_MMU),
        .CP0_LLAddr_rw_bitmask = 0xFFFFFFFFL,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 16,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x3678FFFF,
        /* The VR5432 has a full 64bit FPU but doesn't use the fcr0 bits. */
        .CP1_fcr0 = (0x54 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 40,
        .PABITS = 32,
        .insn_flags = CPU_VR54XX,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "5Kc",
        .CP0_PRid = 0x00018100,
        .CP0_Config0 = MIPS_CONFIG0 | (0x2 << CP0C0_AT) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (31 << CP0C1_MMU) |
                       (1 << CP0C1_IS) | (4 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (1 << CP0C1_DS) | (4 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x32F8FFFF,
        .SEGBITS = 42,
        .PABITS = 36,
        .insn_flags = CPU_MIPS64,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "5Kf",
        .CP0_PRid = 0x00018100,
        .CP0_Config0 = MIPS_CONFIG0 | (0x2 << CP0C0_AT) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (31 << CP0C1_MMU) |
                       (1 << CP0C1_IS) | (4 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (1 << CP0C1_DS) | (4 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x36F8FFFF,
        /* The 5Kf has F64 / L / W but doesn't use the fcr0 bits. */
        .CP1_fcr0 = (1 << FCR0_D) | (1 << FCR0_S) |
                    (0x81 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 42,
        .PABITS = 36,
        .insn_flags = CPU_MIPS64,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "20Kc",
        /* We emulate a later version of the 20Kc, earlier ones had a broken
           WAIT instruction. */
        .CP0_PRid = 0x000182a0,
        .CP0_Config0 = MIPS_CONFIG0 | (0x2 << CP0C0_AT) |
                    (MMU_TYPE_R4000 << CP0C0_MT) | (1 << CP0C0_VI),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (47 << CP0C1_MMU) |
                       (2 << CP0C1_IS) | (4 << CP0C1_IL) | (3 << CP0C1_IA) |
                       (2 << CP0C1_DS) | (4 << CP0C1_DL) | (3 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 0,
        .SYNCI_Step = 32,
        .CCRes = 1,
        .CP0_Status_rw_bitmask = 0x36FBFFFF,
        /* The 20Kc has F64 / L / W but doesn't use the fcr0 bits. */
        .CP1_fcr0 = (1 << FCR0_3D) | (1 << FCR0_PS) |
                    (1 << FCR0_D) | (1 << FCR0_S) |
                    (0x82 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 40,
        .PABITS = 36,
        .insn_flags = CPU_MIPS64 | ASE_MIPS3D,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        /* A generic CPU providing MIPS64 Release 2 features.
           FIXME: Eventually this should be replaced by a real CPU model. */
        .name = "MIPS64R2-generic",
        .CP0_PRid = 0x00010000,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) | (0x2 << CP0C0_AT) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (63 << CP0C1_MMU) |
                       (2 << CP0C1_IS) | (4 << CP0C1_IL) | (3 << CP0C1_IA) |
                       (2 << CP0C1_DS) | (4 << CP0C1_DL) | (3 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (1 << CP0C3_LPA),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 0,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x36FBFFFF,
        .CP1_fcr0 = (1 << FCR0_F64) | (1 << FCR0_3D) | (1 << FCR0_PS) |
                    (1 << FCR0_L) | (1 << FCR0_W) | (1 << FCR0_D) |
                    (1 << FCR0_S) | (0x00 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 42,
        /* The architectural limit is 59, but we have hardcoded 36 bit
           in some places...
        .PABITS = 59, */ /* the architectural limit */
        .PABITS = 36,
        .insn_flags = CPU_MIPS64R2 | ASE_MIPS3D,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "Loongson-2E",
        .CP0_PRid = 0x6302,
        /*64KB I-cache and d-cache. 4 way with 32 bit cache line size*/
        .CP0_Config0 = (0x1<<17) | (0x1<<16) | (0x1<<11) | (0x1<<8) | (0x1<<5) |
                       (0x1<<4) | (0x1<<1),
        /* Note: Config1 is only used internally,
                 Loongson-2E has only Config0. */
        .CP0_Config1 = (1 << CP0C1_FP) | (47 << CP0C1_MMU),
        .SYNCI_Step = 16,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x35D0FFFF,
        .CP1_fcr0 = (0x5 << FCR0_PRID) | (0x1 << FCR0_REV),
        .SEGBITS = 40,
        .PABITS = 40,
        .insn_flags = CPU_LOONGSON2E,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
      .name = "Loongson-2F",
      .CP0_PRid = 0x6303,
      /*64KB I-cache and d-cache. 4 way with 32 bit cache line size*/
      .CP0_Config0 = (0x1<<17) | (0x1<<16) | (0x1<<11) | (0x1<<8) | (0x1<<5) |
                     (0x1<<4) | (0x1<<1),
      /* Note: Config1 is only used internally, Loongson-2F has only Config0. */
      .CP0_Config1 = (1 << CP0C1_FP) | (47 << CP0C1_MMU),
      .SYNCI_Step = 16,
      .CCRes = 2,
      .CP0_Status_rw_bitmask = 0xF5D0FF1F,   /*bit5:7 not writable*/
      .CP1_fcr0 = (0x5 << FCR0_PRID) | (0x1 << FCR0_REV),
      .SEGBITS = 40,
      .PABITS = 40,
      .insn_flags = CPU_LOONGSON2F,
      .mmu_type = MMU_TYPE_R4000,
    },

#endif
};

static void mips_cpu_initfn(Object *obj)
{
    MIPSCPU *cpu = MIPS_CPU(obj);
    CPUMIPSState *env = &cpu->env;

    memset(env, 0, sizeof(CPUMIPSState));
    env->cpu_model_str = object_get_typename(obj);
    cpu_exec_init(env);

#ifndef CONFIG_USER_ONLY
    mmu_init(cpu);
#endif
    fpu_init(cpu);
    mvp_init(cpu);
    CPU_CLASS(MIPS_CPU_GET_CLASS(cpu))->reset(CPU(cpu));
}

static void mips_cpu_class_init(ObjectClass *klass, void *data)
{
    CPUClass *cpu_class = CPU_CLASS(klass);
    MIPSCPUClass *k = MIPS_CPU_CLASS(klass);
    const MIPSCPUInfo *info = data;

    k->parent_reset = cpu_class->reset;
    cpu_class->reset = mips_cpu_reset;

    k->cp0_prid                  = info->CP0_PRid;
    k->cp0_config0               = info->CP0_Config0;
    k->cp0_config1               = info->CP0_Config1;
    k->cp0_config2               = info->CP0_Config2;
    k->cp0_config3               = info->CP0_Config3;
    k->cp0_config6               = info->CP0_Config6;
    k->cp0_config7               = info->CP0_Config7;
    k->cp0_lladdr_rw_bitmask     = info->CP0_LLAddr_rw_bitmask;
    k->cp0_lladdr_shift          = info->CP0_LLAddr_shift;
    k->synci_step                = info->SYNCI_Step;
    k->ccres                     = info->CCRes;
    k->cp0_status_rw_bitmask     = info->CP0_Status_rw_bitmask;
    k->cp0_tcstatus_rw_bitmask   = info->CP0_TCStatus_rw_bitmask;
    k->cp0_srsctl                = info->CP0_SRSCtl;
    k->cp1_fcr0                  = info->CP1_fcr0;
    k->segbits                   = info->SEGBITS;
    k->pabits                    = info->PABITS;
    k->cp0_srsconf_rw_bitmask[0] = info->CP0_SRSConf0_rw_bitmask;
    k->cp0_srsconf_rw_bitmask[1] = info->CP0_SRSConf1_rw_bitmask;
    k->cp0_srsconf_rw_bitmask[2] = info->CP0_SRSConf2_rw_bitmask;
    k->cp0_srsconf_rw_bitmask[3] = info->CP0_SRSConf3_rw_bitmask;
    k->cp0_srsconf_rw_bitmask[4] = info->CP0_SRSConf4_rw_bitmask;
    k->cp0_srsconf[0]            = info->CP0_SRSConf0;
    k->cp0_srsconf[1]            = info->CP0_SRSConf1;
    k->cp0_srsconf[2]            = info->CP0_SRSConf2;
    k->cp0_srsconf[3]            = info->CP0_SRSConf3;
    k->cp0_srsconf[4]            = info->CP0_SRSConf4;
    k->insn_flags                = info->insn_flags;
    k->mmu_type                  = info->mmu_type;
}

static void mips_register_cpu(const MIPSCPUInfo *info)
{
    TypeInfo type = {
        .name = info->name,
        .parent = TYPE_MIPS_CPU,
        .instance_size = sizeof(MIPSCPU),
        .instance_init = mips_cpu_initfn,
        .class_size = sizeof(MIPSCPUClass),
        .class_init = mips_cpu_class_init,
        .class_data = (void *)info,
    };

    type_register_static(&type);
}

static const TypeInfo mips_cpu_info = {
    .name = TYPE_MIPS_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(MIPSCPU),
    .abstract = true,
    .class_size = sizeof(MIPSCPUClass),
};

static void mips_cpu_register_types(void)
{
    int i;

    type_register_static(&mips_cpu_info);
    for (i = 0; i < ARRAY_SIZE(mips_cpus); i++) {
        mips_register_cpu(&mips_cpus[i]);
    }
}

type_init(mips_cpu_register_types)
