/*
 * QTest testcase for QOM
 *
 * Copyright (c) 2013 SUSE LINUX Products GmbH
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */
#include "libqtest.h"

#include <glib.h>
#include <string.h>
#include "qemu/osdep.h"

static void test_nop(gconstpointer data)
{
    QTestState *s;
    const char *machine = data;
    char *args;

    args = g_strdup_printf("-display none -machine %s", machine);
    s = qtest_start(args);
    if (s) {
        qtest_quit(s);
    }
    g_free(args);
}

static const char *x86_machines[] = {
    "pc",
    "isapc",
    "q35",
};

static const char *alpha_machines[] = {
    "clipper",
};

static const char *arm_machines[] = {
    "integratorcp",
    "versatilepb",
    "versatileab",
    "lm3s811evb",
    "lm3s6965evb",
    "collie",
    "akita",
    "spitz",
    "borzoi",
    "terrier",
    "tosa",
    "cheetah",
    "sx1-v1",
    "sx1",
    "realview-eb",
    "realview-eb-mpcore",
    "realview-pb-a8",
    "realview-pbx-a9",
    "musicpal",
    "mainstone",
    "connex",
    "verdex",
    "z2",
    "n800",
    "n810",
    "kzm",
    "vexpress-a9",
    "vexpress-a15",
    "smdkc210",
    "nuri",
    "xilinx-zynq-a9",
    "highbank",
    "midway",
};

static const char *cris_machines[] = {
    "axis-dev88",
};

static const char *lm32_machines[] = {
    "lm32-evr",
    "lm32-uclinux",
    "milkymist",
};

static const char *m68k_machines[] = {
    "mcf5208evb",
    "an5206",
    "dummy",
};

static const char *microblaze_machines[] = {
    "petalogix-ml605",
    "petalogix-s3adsp1800",
};

static const char *mips_machines[] = {
    "malta",
    "magnum",
    "mips",
    "mipssim",
    "pica61",
};

static const char *moxie_machines[] = {
    "moxiesim",
};

static const char *openrisc_machines[] = {
    "or32-sim",
};

static const char *ppc_machines[] = {
    "g3beige",
    "mac99",
    "prep",
    "mpc8544ds",
    "ppce500",
};

static const char *ppc405_machines[] = {
    "ref405ep",
    "taihu",
};

static const char *ppc440_machines[] = {
    "bamboo",
    "virtex-ml507",
};

static const char *s390_machines[] = {
    "s390-virtio",
    "s390-ccw-virtio",
};

static const char *superh_machines[] = {
    "r2d",
    "shix",
};

static const char *sparc_machines[] = {
    "SS-4",
    "SS-5",
    "SS-10",
    "SS-20",
    "SS-600MP",
    "LX",
    "SPARCClassic",
    "SPARCbook",
    "leon3_generic",
};

static const char *sparc64_machines[] = {
    "sun4u",
    "sun4v",
    "Niagara",
};

static const char *unicore32_machines[] = {
    "puv3",
};

static const char *xtensa_machines[] = {
    "sim",
    "lx60",
    "lx200",
};

static void add_test_cases(const char *arch, const char *machine)
{
    char *path;
    path = g_strdup_printf("/%s/qom/%s", arch, machine);
    g_test_add_data_func(path, machine, test_nop);
}

int main(int argc, char **argv)
{
    const char *arch = qtest_get_arch();
    int i;

    g_test_init(&argc, &argv, NULL);

    add_test_cases(arch, "none");

    if (strcmp(arch, "i386") == 0 ||
        strcmp(arch, "x86_64") == 0) {
        for (i = 0; i < ARRAY_SIZE(x86_machines); i++) {
            add_test_cases(arch, x86_machines[i]);
        }
    } else if (strcmp(arch, "alpha") == 0) {
        for (i = 0; i < ARRAY_SIZE(alpha_machines); i++) {
            add_test_cases(arch, alpha_machines[i]);
        }
    } else if (strcmp(arch, "arm") == 0) {
        for (i = 0; i < ARRAY_SIZE(arm_machines); i++) {
            add_test_cases(arch, arm_machines[i]);
        }
    } else if (strcmp(arch, "cris") == 0) {
        for (i = 0; i < ARRAY_SIZE(cris_machines); i++) {
            add_test_cases(arch, cris_machines[i]);
        }
    } else if (strcmp(arch, "lm32") == 0) {
        for (i = 0; i < ARRAY_SIZE(lm32_machines); i++) {
            add_test_cases(arch, lm32_machines[i]);
        }
    } else if (strcmp(arch, "m68k") == 0) {
        for (i = 0; i < ARRAY_SIZE(m68k_machines); i++) {
            add_test_cases(arch, m68k_machines[i]);
        }
    } else if (strcmp(arch, "microblaze") == 0 ||
               strcmp(arch, "microblazeel") == 0) {
        for (i = 0; i < ARRAY_SIZE(microblaze_machines); i++) {
            add_test_cases(arch, microblaze_machines[i]);
        }
    } else if (strcmp(arch, "mips") == 0 ||
               strcmp(arch, "mipsel") == 0 ||
               strcmp(arch, "mips64") == 0 ||
               strcmp(arch, "mips64el") == 0) {
        for (i = 0; i < ARRAY_SIZE(mips_machines); i++) {
            add_test_cases(arch, mips_machines[i]);
        }
        if (strcmp(arch, "mips64el") == 0) {
            add_test_cases(arch, "fulong2e");
        }
    } else if (strcmp(arch, "moxie") == 0) {
        for (i = 0; i < ARRAY_SIZE(moxie_machines); i++) {
            add_test_cases(arch, moxie_machines[i]);
        }
    } else if (strcmp(arch, "or32") == 0) {
        for (i = 0; i < ARRAY_SIZE(openrisc_machines); i++) {
            add_test_cases(arch, openrisc_machines[i]);
        }
    } else if (strcmp(arch, "ppc") == 0 ||
               strcmp(arch, "ppc64") == 0 ||
               strcmp(arch, "ppcemb") == 0) {
        if (strcmp(arch, "ppcemb") != 0) {
            /* XXX Currently available in ppcemb but shouldn't be */
            for (i = 0; i < ARRAY_SIZE(ppc_machines); i++) {
                add_test_cases(arch, ppc_machines[i]);
            }
            /* XXX Currently available in ppcemb but don't work */
            for (i = 0; i < ARRAY_SIZE(ppc405_machines); i++) {
                add_test_cases(arch, ppc405_machines[i]);
            }
        }
        for (i = 0; i < ARRAY_SIZE(ppc440_machines); i++) {
            add_test_cases(arch, ppc440_machines[i]);
        }
        if (strcmp(arch, "ppc64") == 0) {
            add_test_cases(arch, "pseries");
        }
    } else if (strcmp(arch, "s390x") == 0) {
        for (i = 0; i < ARRAY_SIZE(s390_machines); i++) {
            add_test_cases(arch, s390_machines[i]);
        }
    } else if (strcmp(arch, "sh4") == 0 ||
               strcmp(arch, "sh4eb") == 0) {
        for (i = 0; i < ARRAY_SIZE(superh_machines); i++) {
            add_test_cases(arch, superh_machines[i]);
        }
    } else if (strcmp(arch, "sparc") == 0) {
        for (i = 0; i < ARRAY_SIZE(sparc_machines); i++) {
            add_test_cases(arch, sparc_machines[i]);
        }
    } else if (strcmp(arch, "sparc64") == 0) {
        for (i = 0; i < ARRAY_SIZE(sparc64_machines); i++) {
            add_test_cases(arch, sparc64_machines[i]);
        }
    } else if (strcmp(arch, "unicore32") == 0) {
        for (i = 0; i < ARRAY_SIZE(unicore32_machines); i++) {
            add_test_cases(arch, unicore32_machines[i]);
        }
    } else if (strcmp(arch, "xtensa") == 0 ||
               strcmp(arch, "xtensaeb") == 0) {
        for (i = 0; i < ARRAY_SIZE(xtensa_machines); i++) {
            add_test_cases(arch, xtensa_machines[i]);
        }
    }

    return g_test_run();
}
