/*
 * x86 gdb server stub
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

#ifdef TARGET_X86_64
static const int gpr_map[16] = {
    R_EAX, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI, R_EBP, R_ESP,
    8, 9, 10, 11, 12, 13, 14, 15
};
#else
#define gpr_map gpr_map32
#endif
static const int gpr_map32[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

#define IDX_IP_REG      CPU_NB_REGS
#define IDX_FLAGS_REG   (IDX_IP_REG + 1)
#define IDX_SEG_REGS    (IDX_FLAGS_REG + 1)
#define IDX_FP_REGS     (IDX_SEG_REGS + 6)
#define IDX_XMM_REGS    (IDX_FP_REGS + 16)
#define IDX_MXCSR_REG   (IDX_XMM_REGS + CPU_NB_REGS)

static int cpu_gdb_read_register(CPUX86State *env, uint8_t *mem_buf, int n)
{
    if (n < CPU_NB_REGS) {
        if (TARGET_LONG_BITS == 64 && env->hflags & HF_CS64_MASK) {
            GET_REG64(env->regs[gpr_map[n]]);
        } else if (n < CPU_NB_REGS32) {
            GET_REG32(env->regs[gpr_map32[n]]);
        }
    } else if (n >= IDX_FP_REGS && n < IDX_FP_REGS + 8) {
#ifdef USE_X86LDOUBLE
        /* FIXME: byteswap float values - after fixing fpregs layout. */
        memcpy(mem_buf, &env->fpregs[n - IDX_FP_REGS], 10);
#else
        memset(mem_buf, 0, 10);
#endif
        return 10;
    } else if (n >= IDX_XMM_REGS && n < IDX_XMM_REGS + CPU_NB_REGS) {
        n -= IDX_XMM_REGS;
        if (n < CPU_NB_REGS32 ||
            (TARGET_LONG_BITS == 64 && env->hflags & HF_CS64_MASK)) {
            stq_p(mem_buf, env->xmm_regs[n].XMM_Q(0));
            stq_p(mem_buf + 8, env->xmm_regs[n].XMM_Q(1));
            return 16;
        }
    } else {
        switch (n) {
        case IDX_IP_REG:
            if (TARGET_LONG_BITS == 64 && env->hflags & HF_CS64_MASK) {
                GET_REG64(env->eip);
            } else {
                GET_REG32(env->eip);
            }
        case IDX_FLAGS_REG:
            GET_REG32(env->eflags);

        case IDX_SEG_REGS:
            GET_REG32(env->segs[R_CS].selector);
        case IDX_SEG_REGS + 1:
            GET_REG32(env->segs[R_SS].selector);
        case IDX_SEG_REGS + 2:
            GET_REG32(env->segs[R_DS].selector);
        case IDX_SEG_REGS + 3:
            GET_REG32(env->segs[R_ES].selector);
        case IDX_SEG_REGS + 4:
            GET_REG32(env->segs[R_FS].selector);
        case IDX_SEG_REGS + 5:
            GET_REG32(env->segs[R_GS].selector);

        case IDX_FP_REGS + 8:
            GET_REG32(env->fpuc);
        case IDX_FP_REGS + 9:
            GET_REG32((env->fpus & ~0x3800) |
                      (env->fpstt & 0x7) << 11);
        case IDX_FP_REGS + 10:
            GET_REG32(0); /* ftag */
        case IDX_FP_REGS + 11:
            GET_REG32(0); /* fiseg */
        case IDX_FP_REGS + 12:
            GET_REG32(0); /* fioff */
        case IDX_FP_REGS + 13:
            GET_REG32(0); /* foseg */
        case IDX_FP_REGS + 14:
            GET_REG32(0); /* fooff */
        case IDX_FP_REGS + 15:
            GET_REG32(0); /* fop */

        case IDX_MXCSR_REG:
            GET_REG32(env->mxcsr);
        }
    }
    return 0;
}

static int cpu_x86_gdb_load_seg(CPUX86State *env, int sreg, uint8_t *mem_buf)
{
    uint16_t selector = ldl_p(mem_buf);

    if (selector != env->segs[sreg].selector) {
#if defined(CONFIG_USER_ONLY)
        cpu_x86_load_seg(env, sreg, selector);
#else
        unsigned int limit, flags;
        target_ulong base;

        if (!(env->cr[0] & CR0_PE_MASK) || (env->eflags & VM_MASK)) {
            base = selector << 4;
            limit = 0xffff;
            flags = 0;
        } else {
            if (!cpu_x86_get_descr_debug(env, selector, &base, &limit,
                                         &flags)) {
                return 4;
            }
        }
        cpu_x86_load_seg_cache(env, sreg, selector, base, limit, flags);
#endif
    }
    return 4;
}

static int cpu_gdb_write_register(CPUX86State *env, uint8_t *mem_buf, int n)
{
    uint32_t tmp;

    if (n < CPU_NB_REGS) {
        if (TARGET_LONG_BITS == 64 && env->hflags & HF_CS64_MASK) {
            env->regs[gpr_map[n]] = ldtul_p(mem_buf);
            return sizeof(target_ulong);
        } else if (n < CPU_NB_REGS32) {
            n = gpr_map32[n];
            env->regs[n] &= ~0xffffffffUL;
            env->regs[n] |= (uint32_t)ldl_p(mem_buf);
            return 4;
        }
    } else if (n >= IDX_FP_REGS && n < IDX_FP_REGS + 8) {
#ifdef USE_X86LDOUBLE
        /* FIXME: byteswap float values - after fixing fpregs layout. */
        memcpy(&env->fpregs[n - IDX_FP_REGS], mem_buf, 10);
#endif
        return 10;
    } else if (n >= IDX_XMM_REGS && n < IDX_XMM_REGS + CPU_NB_REGS) {
        n -= IDX_XMM_REGS;
        if (n < CPU_NB_REGS32 ||
            (TARGET_LONG_BITS == 64 && env->hflags & HF_CS64_MASK)) {
            env->xmm_regs[n].XMM_Q(0) = ldq_p(mem_buf);
            env->xmm_regs[n].XMM_Q(1) = ldq_p(mem_buf + 8);
            return 16;
        }
    } else {
        switch (n) {
        case IDX_IP_REG:
            if (TARGET_LONG_BITS == 64 && env->hflags & HF_CS64_MASK) {
                env->eip = ldq_p(mem_buf);
                return 8;
            } else {
                env->eip &= ~0xffffffffUL;
                env->eip |= (uint32_t)ldl_p(mem_buf);
                return 4;
            }
        case IDX_FLAGS_REG:
            env->eflags = ldl_p(mem_buf);
            return 4;

        case IDX_SEG_REGS:
            return cpu_x86_gdb_load_seg(env, R_CS, mem_buf);
        case IDX_SEG_REGS + 1:
            return cpu_x86_gdb_load_seg(env, R_SS, mem_buf);
        case IDX_SEG_REGS + 2:
            return cpu_x86_gdb_load_seg(env, R_DS, mem_buf);
        case IDX_SEG_REGS + 3:
            return cpu_x86_gdb_load_seg(env, R_ES, mem_buf);
        case IDX_SEG_REGS + 4:
            return cpu_x86_gdb_load_seg(env, R_FS, mem_buf);
        case IDX_SEG_REGS + 5:
            return cpu_x86_gdb_load_seg(env, R_GS, mem_buf);

        case IDX_FP_REGS + 8:
            env->fpuc = ldl_p(mem_buf);
            return 4;
        case IDX_FP_REGS + 9:
            tmp = ldl_p(mem_buf);
            env->fpstt = (tmp >> 11) & 7;
            env->fpus = tmp & ~0x3800;
            return 4;
        case IDX_FP_REGS + 10: /* ftag */
            return 4;
        case IDX_FP_REGS + 11: /* fiseg */
            return 4;
        case IDX_FP_REGS + 12: /* fioff */
            return 4;
        case IDX_FP_REGS + 13: /* foseg */
            return 4;
        case IDX_FP_REGS + 14: /* fooff */
            return 4;
        case IDX_FP_REGS + 15: /* fop */
            return 4;

        case IDX_MXCSR_REG:
            env->mxcsr = ldl_p(mem_buf);
            return 4;
        }
    }
    /* Unrecognised register.  */
    return 0;
}
