/*
 * QTest testcase for virtio-ccw
 *
 * Copyright (c) 2013 SUSE LINUX Products GmbH
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */
#include "libqtest.h"

#include <glib.h>

#define KVM_S390_VIRTIO_RESET           1

static void test(void)
{
    hypercall(KVM_S390_VIRTIO_RESET, 1, 128 * 1024 * 1024);
}

int main(int argc, char **argv)
{
    QTestState *s = NULL;
    int ret;

    g_test_init(&argc, &argv, NULL);

    s = qtest_start("-display none -m 128M");

    qtest_add_func("/virtio-ccw/dummy", test);

    ret = g_test_run();

    if (s) {
        qtest_quit(s);
    }

    return ret;
}
