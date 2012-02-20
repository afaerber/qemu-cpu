/*
 * QEMU Xtensa CPU
 *
 * Copyright (c) 2011, Max Filippov, Open Source and Linux Lab.
 * Copyright (c) 2012 SUSE LINUX Products GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Open Source and Linux Lab nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef QEMU_XTENSA_CPU_QOM_H
#define QEMU_XTENSA_CPU_QOM_H

#include "qemu/cpu.h"
#include "cpu.h"

#define TYPE_XTENSA_CPU "xtensa-cpu"

#define XTENSA_CPU_CLASS(klass) \
    OBJECT_CLASS_CHECK(XtensaCPUClass, (klass), TYPE_XTENSA_CPU)
#define XTENSA_CPU(obj) \
    OBJECT_CHECK(XtensaCPU, (obj), TYPE_XTENSA_CPU)
#define XTENSA_CPU_GET_CLASS(obj) \
    OBJECT_GET_CLASS(XtensaCPUClass, (obj), TYPE_XTENSA_CPU)

typedef struct xtensa_tlb {
    unsigned nways;
    const unsigned way_size[10];
    bool varway56;
    unsigned nrefillentries;
} xtensa_tlb;

typedef struct XtensaGdbReg {
    int targno;
    int type;
    int group;
} XtensaGdbReg;

typedef struct XtensaGdbRegmap {
    int num_regs;
    int num_core_regs;
    /* PC + a + ar + sr + ur */
    XtensaGdbReg reg[1 + 16 + 64 + 256 + 256];
} XtensaGdbRegmap;

/**
 * XtensaCPUClass:
 * @parent_reset: The parent class' reset handler.
 *
 * An Xtensa CPU model.
 */
typedef struct XtensaCPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/

    void (*parent_reset)(CPUState *cpu);

    uint64_t options;
    XtensaGdbRegmap gdb_regmap;
    unsigned nareg;
    int excm_level;
    int ndepc;
    uint32_t vecbase;
    uint32_t exception_vector[EXC_MAX];
    unsigned ninterrupt;
    unsigned nlevel;
    uint32_t interrupt_vector[MAX_NLEVEL + MAX_NNMI + 1];
    uint32_t level_mask[MAX_NLEVEL + MAX_NNMI + 1];
    uint32_t inttype_mask[INTTYPE_MAX];
    struct {
        uint32_t level;
        interrupt_type inttype;
    } interrupt[MAX_NINTERRUPT];
    unsigned nccompare;
    uint32_t timerint[MAX_NCCOMPARE];
    unsigned nextint;
    unsigned extint[MAX_NINTERRUPT];

    unsigned debug_level;
    unsigned nibreak;
    unsigned ndbreak;

    uint32_t clock_freq_khz;

    xtensa_tlb itlb;
    xtensa_tlb dtlb;
} XtensaCPUClass;

/**
 * XtensaCPU:
 * @env: Legacy CPU state.
 *
 * An Xtensa CPU.
 */
typedef struct XtensaCPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUXtensaState env;
} XtensaCPU;

static inline XtensaCPU *xtensa_env_get_cpu(const CPUXtensaState *env)
{
    return XTENSA_CPU(container_of(env, XtensaCPU, env));
}

#define ENV_GET_CPU(e) CPU(xtensa_env_get_cpu(e))

#define ENV_OFFSET offsetof(XtensaCPU, env)


#define XTENSA_OPTION_BIT(opt) (((uint64_t)1) << (opt))

static inline bool xtensa_option_bits_enabled(XtensaCPUClass *klass,
                                              uint64_t opt)
{
    return (klass->options & opt) != 0;
}

static inline bool xtensa_option_enabled(XtensaCPUClass *klass, int opt)
{
    return xtensa_option_bits_enabled(klass, XTENSA_OPTION_BIT(opt));
}

static inline bool xtensa_cpu_option_enabled(XtensaCPU *cpu, int opt)
{
    return xtensa_option_enabled(XTENSA_CPU_GET_CLASS(cpu), opt);
}

static inline int xtensa_get_cintlevel(XtensaCPU *cpu)
{
    XtensaCPUClass *klass = XTENSA_CPU_GET_CLASS(cpu);
    CPUXtensaState *env = &cpu->env;
    int level = (env->sregs[PS] & PS_INTLEVEL) >> PS_INTLEVEL_SHIFT;
    if ((env->sregs[PS] & PS_EXCM) && klass->excm_level > level) {
        level = klass->excm_level;
    }
    return level;
}

static inline int xtensa_get_debug_level(XtensaCPU *cpu)
{
    XtensaCPUClass *klass = XTENSA_CPU_GET_CLASS(cpu);
    return klass->debug_level;
}

static inline int xtensa_get_ring(XtensaCPU *cpu)
{
    if (xtensa_cpu_option_enabled(cpu, XTENSA_OPTION_MMU)) {
        return (cpu->env.sregs[PS] & PS_RING) >> PS_RING_SHIFT;
    } else {
        return 0;
    }
}

static inline int xtensa_get_cring(XtensaCPU *cpu)
{
    if (xtensa_cpu_option_enabled(cpu, XTENSA_OPTION_MMU) &&
            (cpu->env.sregs[PS] & PS_EXCM) == 0) {
        return (cpu->env.sregs[PS] & PS_RING) >> PS_RING_SHIFT;
    } else {
        return 0;
    }
}


#endif
