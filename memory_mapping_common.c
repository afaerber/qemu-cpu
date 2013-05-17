/*
 * QEMU memory mapping
 *
 * Copyright Fujitsu, Corp. 2011, 2012
 *
 * Authors:
 *     Wen Congyang <wency@cn.fujitsu.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#include "cpu.h"
#include "exec/cpu-all.h"
#include "sysemu/memory_mapping.h"

void memory_mapping_list_add_mapping_sorted(MemoryMappingList *list,
                                            MemoryMapping *mapping)
{
    MemoryMapping *p;

    QTAILQ_FOREACH(p, &list->head, next) {
        if (p->phys_addr >= mapping->phys_addr) {
            QTAILQ_INSERT_BEFORE(p, mapping, next);
            return;
        }
    }
    QTAILQ_INSERT_TAIL(&list->head, mapping, next);
}

void create_new_memory_mapping(MemoryMappingList *list,
                               hwaddr phys_addr,
                               hwaddr virt_addr,
                               ram_addr_t length)
{
    MemoryMapping *memory_mapping;

    memory_mapping = g_malloc(sizeof(MemoryMapping));
    memory_mapping->phys_addr = phys_addr;
    memory_mapping->virt_addr = virt_addr;
    memory_mapping->length = length;
    list->last_mapping = memory_mapping;
    list->num++;
    memory_mapping_list_add_mapping_sorted(list, memory_mapping);
}


void memory_mapping_list_free(MemoryMappingList *list)
{
    MemoryMapping *p, *q;

    QTAILQ_FOREACH_SAFE(p, &list->head, next, q) {
        QTAILQ_REMOVE(&list->head, p, next);
        g_free(p);
    }

    list->num = 0;
    list->last_mapping = NULL;
}

void memory_mapping_list_init(MemoryMappingList *list)
{
    list->num = 0;
    list->last_mapping = NULL;
    QTAILQ_INIT(&list->head);
}

void qemu_get_guest_simple_memory_mapping(MemoryMappingList *list)
{
    RAMBlock *block;

    QTAILQ_FOREACH(block, &ram_list.blocks, next) {
        create_new_memory_mapping(list, block->offset, 0, block->length);
    }
}

void memory_mapping_filter(MemoryMappingList *list, int64_t begin,
                           int64_t length)
{
    MemoryMapping *cur, *next;

    QTAILQ_FOREACH_SAFE(cur, &list->head, next, next) {
        if (cur->phys_addr >= begin + length ||
            cur->phys_addr + cur->length <= begin) {
            QTAILQ_REMOVE(&list->head, cur, next);
            list->num--;
            continue;
        }

        if (cur->phys_addr < begin) {
            cur->length -= begin - cur->phys_addr;
            if (cur->virt_addr) {
                cur->virt_addr += begin - cur->phys_addr;
            }
            cur->phys_addr = begin;
        }

        if (cur->phys_addr + cur->length > begin + length) {
            cur->length -= cur->phys_addr + cur->length - begin - length;
        }
    }
}
