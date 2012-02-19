/*
 *  LatticeMico32 helper routines.
 *
 *  Copyright (c) 2010 Michael Walle <michael@walle.cc>
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

#include "cpu.h"
#include "host-utils.h"

int cpu_lm32_handle_mmu_fault(CPULM32State *env, target_ulong address, int rw,
                              int mmu_idx)
{
    int prot;

    address &= TARGET_PAGE_MASK;
    prot = PAGE_BITS;
    if (env->flags & LM32_FLAG_IGNORE_MSB) {
        tlb_set_page(env, address, address & 0x7fffffff, prot, mmu_idx,
                TARGET_PAGE_SIZE);
    } else {
        tlb_set_page(env, address, address, prot, mmu_idx, TARGET_PAGE_SIZE);
    }

    return 0;
}

target_phys_addr_t cpu_get_phys_page_debug(CPULM32State *env, target_ulong addr)
{
    return addr & TARGET_PAGE_MASK;
}

void do_interrupt(CPULM32State *env)
{
    qemu_log_mask(CPU_LOG_INT,
            "exception at pc=%x type=%x\n", env->pc, env->exception_index);

    switch (env->exception_index) {
    case EXCP_INSN_BUS_ERROR:
    case EXCP_DATA_BUS_ERROR:
    case EXCP_DIVIDE_BY_ZERO:
    case EXCP_IRQ:
    case EXCP_SYSTEMCALL:
        /* non-debug exceptions */
        env->regs[R_EA] = env->pc;
        env->ie |= (env->ie & IE_IE) ? IE_EIE : 0;
        env->ie &= ~IE_IE;
        if (env->dc & DC_RE) {
            env->pc = env->deba + (env->exception_index * 32);
        } else {
            env->pc = env->eba + (env->exception_index * 32);
        }
        log_cpu_state_mask(CPU_LOG_INT, env, 0);
        break;
    case EXCP_BREAKPOINT:
    case EXCP_WATCHPOINT:
        /* debug exceptions */
        env->regs[R_BA] = env->pc;
        env->ie |= (env->ie & IE_IE) ? IE_BIE : 0;
        env->ie &= ~IE_IE;
        env->pc = env->deba + (env->exception_index * 32);
        log_cpu_state_mask(CPU_LOG_INT, env, 0);
        break;
    default:
        cpu_abort(env, "unhandled exception type=%d\n",
                  env->exception_index);
        break;
    }
}

typedef struct LM32CPUListState {
    fprintf_function cpu_fprintf;
    FILE *file;
} LM32CPUListState;

/* Sort alphabetically. */
static gint lm32_cpu_list_compare(gconstpointer a, gconstpointer b)
{
    ObjectClass *class_a = OBJECT_CLASS(a);
    ObjectClass *class_b = OBJECT_CLASS(b);

    return strcasecmp(object_class_get_name(class_a),
                      object_class_get_name(class_b));
}

static void lm32_cpu_list_entry(gpointer data, gpointer user_data)
{
    ObjectClass *klass = data;
    LM32CPUListState *s = user_data;

    (*s->cpu_fprintf)(s->file, "  %s\n",
                      object_class_get_name(klass));
}

void cpu_lm32_list(FILE *f, fprintf_function cpu_fprintf)
{
    LM32CPUListState s = {
        .file = f,
        .cpu_fprintf = cpu_fprintf,
    };
    GSList *list;

    list = object_class_get_list(TYPE_LM32_CPU, false);
    list = g_slist_sort(list, lm32_cpu_list_compare);
    cpu_fprintf(f, "Available CPUs:\n");
    g_slist_foreach(list, lm32_cpu_list_entry, &s);
    g_slist_free(list);
}

CPULM32State *cpu_lm32_init(const char *cpu_model)
{
    LM32CPU *cpu;
    CPULM32State *env;
    static int tcg_initialized;

    if (object_class_by_name(cpu_model) == NULL) {
        return NULL;
    }
    cpu = LM32_CPU(object_new(cpu_model));
    env = &cpu->env;

    qemu_init_vcpu(env);

    if (!tcg_initialized) {
        tcg_initialized = 1;
        lm32_translate_init();
    }

    return env;
}

/* Some soc ignores the MSB on the address bus. Thus creating a shadow memory
 * area. As a general rule, 0x00000000-0x7fffffff is cached, whereas
 * 0x80000000-0xffffffff is not cached and used to access IO devices. */
void cpu_lm32_set_phys_msb_ignore(CPULM32State *env, int value)
{
    if (value) {
        env->flags |= LM32_FLAG_IGNORE_MSB;
    } else {
        env->flags &= ~LM32_FLAG_IGNORE_MSB;
    }
}

void cpu_state_reset(CPULM32State *env)
{
    if (qemu_loglevel_mask(CPU_LOG_RESET)) {
        qemu_log("CPU Reset (CPU %d)\n", env->cpu_index);
        log_cpu_state(env, 0);
    }

    tlb_flush(env, 1);

    /* reset cpu state */
    memset(env, 0, offsetof(CPULM32State, breakpoints));
}

