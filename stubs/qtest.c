#include "qemu-common.h"
#include "sysemu/qtest.h"

bool qtest_hypercall_supported(void)
{
    return false;
}

int qtest_hypercall(uint64_t code, uint64_t *args)
{
    return -EINVAL;
}
