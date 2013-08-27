#include <assert.h>
#include "cpu.h"
#include "helper.h"
#include "qemu/host-utils.h"

#include "hw/lm32/lm32_pic.h"
#include "hw/char/lm32_juart.h"

#include "exec/softmmu_exec.h"

#if !defined(CONFIG_USER_ONLY)
#define MMUSUFFIX _mmu
#define SHIFT 0
#include "exec/softmmu_template.h"
#define SHIFT 1
#include "exec/softmmu_template.h"
#define SHIFT 2
#include "exec/softmmu_template.h"
#define SHIFT 3
#include "exec/softmmu_template.h"

void HELPER(raise_exception)(CPULM32State *env, uint32_t index)
{
    CPUState *cs = CPU(lm32_env_get_cpu(env));

    cs->exception_index = index;
    cpu_loop_exit(cs);
}

void HELPER(hlt)(CPULM32State *env)
{
    CPUState *cs = CPU(lm32_env_get_cpu(env));

    cs->halted = 1;
    cs->exception_index = EXCP_HLT;
    cpu_loop_exit(cs);
}

void HELPER(wcsr_im)(CPULM32State *env, uint32_t im)
{
    lm32_pic_set_im(env->pic_state, im);
}

void HELPER(wcsr_ip)(CPULM32State *env, uint32_t im)
{
    lm32_pic_set_ip(env->pic_state, im);
}

void HELPER(wcsr_jtx)(CPULM32State *env, uint32_t jtx)
{
    lm32_juart_set_jtx(env->juart_state, jtx);
}

void HELPER(wcsr_jrx)(CPULM32State *env, uint32_t jrx)
{
    lm32_juart_set_jrx(env->juart_state, jrx);
}

uint32_t HELPER(rcsr_im)(CPULM32State *env)
{
    return lm32_pic_get_im(env->pic_state);
}

uint32_t HELPER(rcsr_ip)(CPULM32State *env)
{
    return lm32_pic_get_ip(env->pic_state);
}

uint32_t HELPER(rcsr_jtx)(CPULM32State *env)
{
    return lm32_juart_get_jtx(env->juart_state);
}

uint32_t HELPER(rcsr_jrx)(CPULM32State *env)
{
    return lm32_juart_get_jrx(env->juart_state);
}

/* Try to fill the TLB and return an exception if error. If retaddr is
 * NULL, it means that the function was called in C code (i.e. not
 * from generated code or from helper.c)
 */
void tlb_fill(CPUState *cs, target_ulong addr, int is_write, int mmu_idx,
              uintptr_t retaddr)
{
    int ret;

    ret = lm32_cpu_handle_mmu_fault(cs, addr, is_write, mmu_idx);
    if (unlikely(ret)) {
        LM32CPU *cpu = LM32_CPU(cs);
        CPULM32State *env = &cpu->env;

        if (retaddr) {
            /* now we have a real cpu fault */
            cpu_restore_state(env, retaddr);
        }
        cpu_loop_exit(cs);
    }
}
#endif

