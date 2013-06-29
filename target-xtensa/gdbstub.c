/*
 * Xtensa gdb server stub
 *
 * Copyright (c) 2003-2005 Fabrice Bellard
 * Copyright (c) 2013 SUSE LINUX Products GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"
#include "qemu-common.h"
#include "exec/gdbstub.h"

int xtensa_cpu_gdb_read_register(CPUState *cs, uint8_t *mem_buf, int n)
{
    XtensaCPU *cpu = XTENSA_CPU(cs);
    XtensaCPUClass *xcc = XTENSA_CPU_GET_CLASS(cs);
    CPUXtensaState *env = &cpu->env;
    const XtensaGdbReg *reg = xcc->config->gdb_regmap.reg + n;

    if (n < 0 || n >= xcc->config->gdb_regmap.num_regs) {
        return 0;
    }

    switch (reg->type) {
    case 9: /*pc*/
        return gdb_get_reg32(mem_buf, env->pc);

    case 1: /*ar*/
        xtensa_sync_phys_from_window(env);
        return gdb_get_reg32(mem_buf, env->phys_regs[(reg->targno & 0xff)
                                                     % xcc->config->nareg]);

    case 2: /*SR*/
        return gdb_get_reg32(mem_buf, env->sregs[reg->targno & 0xff]);

    case 3: /*UR*/
        return gdb_get_reg32(mem_buf, env->uregs[reg->targno & 0xff]);

    case 4: /*f*/
        return gdb_get_reg32(mem_buf,
                             float32_val(env->fregs[reg->targno & 0x0f]));

    case 8: /*a*/
        return gdb_get_reg32(mem_buf, env->regs[reg->targno & 0x0f]);

    default:
        qemu_log("%s from reg %d of unsupported type %d\n",
                __func__, n, reg->type);
        return 0;
    }
}

int xtensa_cpu_gdb_write_register(CPUState *cs, uint8_t *mem_buf, int n)
{
    XtensaCPU *cpu = XTENSA_CPU(cs);
    XtensaCPUClass *xcc = XTENSA_CPU_GET_CLASS(cs);
    CPUXtensaState *env = &cpu->env;
    uint32_t tmp;
    const XtensaGdbReg *reg = xcc->config->gdb_regmap.reg + n;

    if (n < 0 || n >= xcc->config->gdb_regmap.num_regs) {
        return 0;
    }

    tmp = ldl_p(mem_buf);

    switch (reg->type) {
    case 9: /*pc*/
        env->pc = tmp;
        break;

    case 1: /*ar*/
        env->phys_regs[(reg->targno & 0xff) % xcc->config->nareg] = tmp;
        xtensa_sync_window_from_phys(env);
        break;

    case 2: /*SR*/
        env->sregs[reg->targno & 0xff] = tmp;
        break;

    case 3: /*UR*/
        env->uregs[reg->targno & 0xff] = tmp;
        break;

    case 4: /*f*/
        env->fregs[reg->targno & 0x0f] = make_float32(tmp);
        break;

    case 8: /*a*/
        env->regs[reg->targno & 0x0f] = tmp;
        break;

    default:
        qemu_log("%s to reg %d of unsupported type %d\n",
                __func__, n, reg->type);
        return 0;
    }

    return 4;
}
