#include "arch/instr.h"
#include "libk/kassert.h"
#include "libk/kprintf.h"
#include "types.h"

// All PCI specification references reference PCI Local Bus Specification 3.0.

// All PCI devices (except host bus bridges) must implement configuration space.
// A device's configuration space must be accessible at all times, not just
// during system boot.

// The first 16 bytes are defined the same for all types of devices. The
// remaining bytes can have different layouts depending on the base fuction that
// the device supports.

// See PCI specification 3.2.2.3.2
#define PORT_CONFIG_ADDR 0xcf8
#define PORT_CONFIG_DATA 0xcfc

static inline u32 config_addr(u8 bus, u8 device, u8 function, u8 offset) {
    kassert(device < 32);
    kassert(function < 8);
    kassert(offset % 4 == 0);

    // See PCI specification 3.2.2.3.1
    return (1U << 31) | ((u32)bus << 16) | ((u32)(device & 0x1f) << 11)
        | ((u32)(function & 0x7) << 8) | (offset & 0b11111100);
}

static inline u32 pci_read_config(u8 bus, u8 device, u8 function, u8 offset) {
    outd(PORT_CONFIG_ADDR, config_addr(bus, device, function, offset));
    return ind(PORT_CONFIG_DATA);
}

// 6.2.1

// Configuration space organization is described in 6.1

void pci_list(void) {
    u32 config_data;

    for (u8 device = 0; device < 32; ++device) {
        config_data = pci_read_config(0, device, 0, 0);

        u16 vendor_id = config_data & 0xffff;
        if (vendor_id == 0xffff) {
            continue;
        }

        // Note: Unimplemented Base Address registers are hardwired to zero.

        config_data = pci_read_config(0, device, 0, 8);
        kprintf("00:%02x.%d class code: %06x\n", device, 0, config_data >> 8);
        kprintf("0x%08x\n", pci_read_config(0, device, 0, 0x10));

        config_data = pci_read_config(0, device, 0, 0xc);
        u8 header_type = (config_data >> 16) & 0xff;
        // Check for multi-function device
        if (header_type & (1 << 7)) {
            for (u8 function = 1; function < 8; ++function) {
                config_data = pci_read_config(0, device, function, 0);

                u16 func_vendor_id = config_data & 0xffff;
                if (func_vendor_id == 0xffff) {
                    continue;
                }

                config_data = pci_read_config(0, device, function, 8);
                kprintf("00:%02x.%d class code: %06x\n", device, function,
                        config_data >> 8);
                kprintf("0x%08x\n", pci_read_config(0, device, function, 0x10));
            }
        }
    }
}
