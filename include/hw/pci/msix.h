#ifndef QEMU_MSIX_H
#define QEMU_MSIX_H

#include "qemu-common.h"
#include "hw/pci/pci.h"

void msix_set_message(PCIDevice *dev, int vector, MSIMessage msg);
MSIMessage msix_get_message(PCIDevice *dev, unsigned int vector);
int msix_init(PCIDevice *dev, unsigned short nentries,
              MemoryRegion *table_bar, uint8_t table_bar_nr,
              unsigned table_offset, MemoryRegion *pba_bar,
              uint8_t pba_bar_nr, unsigned pba_offset, uint8_t cap_pos);
int msix_init_exclusive_bar(PCIDevice *dev, unsigned short nentries,
                            uint8_t bar_nr);

void msix_write_config(PCIDevice *dev, uint32_t address, uint32_t val, int len);

void msix_uninit(PCIDevice *dev, MemoryRegion *table_bar,
                 MemoryRegion *pba_bar);
void msix_uninit_exclusive_bar(PCIDevice *dev);

unsigned int msix_nr_vectors_allocated(const PCIDevice *dev);

void msix_save(PCIDevice *dev, QEMUFile *f);
void msix_load(PCIDevice *dev, QEMUFile *f);

int msix_enabled(PCIDevice *dev);
int msix_present(PCIDevice *dev);

bool msix_is_masked(PCIDevice *dev, unsigned vector);
void msix_set_pending(PCIDevice *dev, unsigned vector);

int msix_vector_use(PCIDevice *dev, unsigned vector);
void msix_vector_unuse(PCIDevice *dev, unsigned vector);
void msix_unuse_all_vectors(PCIDevice *dev);

void msix_notify(PCIDevice *dev, unsigned vector);

void msix_reset(PCIDevice *dev);

int msix_set_vector_notifiers(PCIDevice *dev,
                              MSIVectorUseNotifier use_notifier,
                              MSIVectorReleaseNotifier release_notifier,
                              MSIVectorPollNotifier poll_notifier);
void msix_unset_vector_notifiers(PCIDevice *dev);

extern const VMStateDescription vmstate_msix;

#define VMSTATE_MSIX_TEST(_test) {                                   \
    .name       = "MSI-X",                                           \
    .size       = sizeof(PCIDevice),                                 \
    .vmsd       = &vmstate_msix,                                     \
    .flags      = VMS_STRUCT,                                        \
    .offset     = 0,                                                 \
    .field_exists = (_test),                                         \
}

#endif
