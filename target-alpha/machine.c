#include "hw/hw.h"
#include "hw/boards.h"

static int get_fpcr(QEMUFile *f, void *opaque, size_t size)
{
    AlphaCPU *cpu = opaque;
    CPUAlphaState *env = &cpu->env;

    cpu_alpha_store_fpcr(env, qemu_get_be64(f));
    return 0;
}

static void put_fpcr(QEMUFile *f, void *opaque, size_t size)
{
    AlphaCPU *cpu = opaque;
    CPUAlphaState *env = &cpu->env;

    qemu_put_be64(f, cpu_alpha_load_fpcr(env));
}

static const VMStateInfo vmstate_fpcr = {
    .name = "fpcr",
    .get = get_fpcr,
    .put = put_fpcr,
};

static VMStateField vmstate_cpu_fields[] = {
    VMSTATE_UINTTL_ARRAY(env.ir, AlphaCPU, 31),
    VMSTATE_UINTTL_ARRAY(env.fir, AlphaCPU, 31),
    /* Save the architecture value of the fpcr, not the internally
       expanded version.  Since this architecture value does not
       exist in memory to be stored, this requires a but of hoop
       jumping.  We want OFFSET=0 so that we effectively pass ENV
       to the helper functions, and we need to fill in the name by
       hand since there's no field of that name.  */
    {
        .name = "fpcr",
        .version_id = 0,
        .size = sizeof(uint64_t),
        .info = &vmstate_fpcr,
        .flags = VMS_SINGLE,
        .offset = 0
    },
    VMSTATE_UINTTL(env.pc, AlphaCPU),
    VMSTATE_UINTTL(env.unique, AlphaCPU),
    VMSTATE_UINTTL(env.lock_addr, AlphaCPU),
    VMSTATE_UINTTL(env.lock_value, AlphaCPU),
    /* Note that lock_st_addr is not saved; it is a temporary
       used during the execution of the st[lq]_c insns.  */

    VMSTATE_UINT8(env.ps, AlphaCPU),
    VMSTATE_UINT8(env.intr_flag, AlphaCPU),
    VMSTATE_UINT8(env.pal_mode, AlphaCPU),
    VMSTATE_UINT8(env.fen, AlphaCPU),

    VMSTATE_UINT32(env.pcc_ofs, AlphaCPU),

    VMSTATE_UINTTL(env.trap_arg0, AlphaCPU),
    VMSTATE_UINTTL(env.trap_arg1, AlphaCPU),
    VMSTATE_UINTTL(env.trap_arg2, AlphaCPU),

    VMSTATE_UINTTL(env.exc_addr, AlphaCPU),
    VMSTATE_UINTTL(env.palbr, AlphaCPU),
    VMSTATE_UINTTL(env.ptbr, AlphaCPU),
    VMSTATE_UINTTL(env.vptptr, AlphaCPU),
    VMSTATE_UINTTL(env.sysval, AlphaCPU),
    VMSTATE_UINTTL(env.usp, AlphaCPU),

    VMSTATE_UINTTL_ARRAY(env.shadow, AlphaCPU, 8),
    VMSTATE_UINTTL_ARRAY(env.scratch, AlphaCPU, 24),

    VMSTATE_END_OF_LIST()
};

const VMStateDescription vmstate_alpha_cpu = {
    .name = "cpu",
    .version_id = 1,
    .minimum_version_id = 1,
    .minimum_version_id_old = 1,
    .fields = vmstate_cpu_fields,
};
