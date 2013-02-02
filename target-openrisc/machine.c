/*
 * OpenRISC Machine
 *
 * Copyright (c) 2011-2012 Jia Liu <proljc@gmail.com>
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

#include "hw/hw.h"
#include "hw/boards.h"

const VMStateDescription vmstate_openrisc_cpu = {
    .name = "cpu",
    .version_id = 1,
    .minimum_version_id = 1,
    .minimum_version_id_old = 1,
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(env.gpr, OpenRISCCPU, 32),
        VMSTATE_UINT32(env.sr, OpenRISCCPU),
        VMSTATE_UINT32(env.epcr, OpenRISCCPU),
        VMSTATE_UINT32(env.eear, OpenRISCCPU),
        VMSTATE_UINT32(env.esr, OpenRISCCPU),
        VMSTATE_UINT32(env.fpcsr, OpenRISCCPU),
        VMSTATE_UINT32(env.pc, OpenRISCCPU),
        VMSTATE_UINT32(env.npc, OpenRISCCPU),
        VMSTATE_UINT32(env.ppc, OpenRISCCPU),
        VMSTATE_END_OF_LIST()
    }
};
