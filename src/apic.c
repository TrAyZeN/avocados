#include "apic.h"
#include "instr.h"
#include "kassert.h"
#include "kprintf.h"
#include "log.h"
#include "regs.h"
#include "vmm.h"

#define LAPIC_VIRT_ADDR 0x0000001100000000UL
#define IOAPIC_VIRT_ADDR 0x0000001100001000UL

// All 32-bit registers should be accessed using 128-bit aligned 32-bit
// loads or stores. (See Vol. 3A 11.4.1)
#define REG_LAPIC_ID ((volatile u32 *)(LAPIC_VIRT_ADDR + 0x20))
#define REG_EOI ((volatile u32 *)(LAPIC_VIRT_ADDR + 0xb0))
#define REG_SVR ((volatile u32 *)(LAPIC_VIRT_ADDR + 0xf0))
#define REG_LVT_LINT0 ((volatile u32 *)(LAPIC_VIRT_ADDR + 0x350))
#define REG_LVT_LINT1 ((volatile u32 *)(LAPIC_VIRT_ADDR + 0x360))

// Same between lapic and ioapic
#define DELIVERY_MODE_FIXED 0b000
#define DELIVERY_MODE_SMI 0b010
#define DELIVERY_MODE_NMI 0b100
#define DELIVERY_MODE_EXT_INT 0b111
#define DELIVERY_MODE_INIT 0b101

#define POLARITY_ACTIVE_HIGH 0
#define POLARITY_ACTIVE_LOW 1

#define TRIGGER_MODE_EDGE 0
#define TRIGGER_MODE_LEVEL 1

#define LVT_LINT(VECTOR, DELIVERY_MODE, POLARITY, TRIGGER_MODE, MASK)          \
    (((VECTOR)&0xff) | (((DELIVERY_MODE)&0b111) << 8) | (((POLARITY)&1) << 13) \
     | (((TRIGGER_MODE)&1) << 15) | (((MASK)&1) << 16))

// TODO: Check bit 24 of Local APIC version register for EOI broadcast
// suppression
#define SVR(VECTOR, SOFT_ENABLE_APIC)                                          \
    (((VECTOR)&0xff) | (((SOFT_ENABLE_APIC)&1) << 8))

#define REG_IOREGSEL ((volatile u32 *)(IOAPIC_VIRT_ADDR + 0x00))
#define REG_IOWIN ((volatile u32 *)(IOAPIC_VIRT_ADDR + 0x10))

#define IOAPICID 0x00
#define IOAPICVER 0x01
#define IOREDTBL_LO(N) (0x10U + (N)*2)
#define IOREDTBL_HI(N) (0x11U + (N)*2)

static void disable_dual_8259a_pic(void);
static inline u32 ioapic_read_register(u32 offset);
static void ioapic_set_io_redirection_table(u8 num, u8 vector, u8 delivery_mode,
                                            u8 destination_mode, u8 polarity,
                                            u8 trigger_mode, u8 mask,
                                            u8 destination);

// On error panic
void apic_init(u64 lapic_phys_addr, u64 ioapic_phys_addr, bool has_8259a) {
    // TODO: Figure out the following
    // For correct APIC operation, this address space must be mapped to an area
    // of memory that has been designated as strong uncacheable (UC)
    // See Vol. 3A 11.4.1

    u64 ia32_apic_base = rdmsr(MSR_IA32_APIC_BASE);
    if (!(ia32_apic_base & (1 << 11))) {
        kpanic("APIC is global disabled\n");
    }
    kassert((ia32_apic_base & 0xfffff000) == lapic_phys_addr);

    u64 res = vmm_map_physical(LAPIC_VIRT_ADDR, lapic_phys_addr, 4096, 0);
    if (res == VMM_ALLOC_ERROR) {
        kpanic("Failed to map local APIC registers\n");
    }
    log(LOG_LEVEL_DEBUG, "APIC: Local APIC mapped\n");

    // If PCAT_COMPAT the 8259 vectors must be disabled when enabling the
    // ACPI APIC operation
    if (has_8259a) {
        disable_dual_8259a_pic();
    }

    // TODO: How do we handle nmi ?
    *REG_LVT_LINT0 = LVT_LINT(VECTOR_NUMBER_LINT0, DELIVERY_MODE_FIXED,
                              POLARITY_ACTIVE_HIGH, TRIGGER_MODE_EDGE, 0);
    // Software should always set the trigger mode in the LVT LINT1 register to
    // edge sensitive. (See Vol. 3A 11.5.1)
    *REG_LVT_LINT1 = LVT_LINT(VECTOR_NUMBER_LINT1, DELIVERY_MODE_FIXED,
                              POLARITY_ACTIVE_HIGH, TRIGGER_MODE_EDGE, 0);
    // Setup spurious interrupt and software enable APIC
    *REG_SVR = SVR(VECTOR_NUMBER_SPURIOUS_INT, 1);

    res = vmm_map_physical(IOAPIC_VIRT_ADDR, ioapic_phys_addr, 4096, 0);
    if (res == VMM_ALLOC_ERROR) {
        kpanic("Failed to map IO APIC registers\n");
    }
    log(LOG_LEVEL_DEBUG, "APIC: IO APIC mapped\n");

    u32 ioapic_num_entries = (ioapic_read_register(IOAPICVER) >> 16) & 0xff;
    log(LOG_LEVEL_DEBUG, "APIC: number of IO APIC entries: %u\n",
        ioapic_num_entries);

    // Mask all interrupts
    for (u8 i = 0; i < ioapic_num_entries; ++i) {
        ioapic_set_io_redirection_table(
            i, VECTOR_NUMBER_IOAPIC + i, DELIVERY_MODE_FIXED, 0,
            POLARITY_ACTIVE_HIGH, TRIGGER_MODE_EDGE, 0, 1);
    }

    // From ACPI:
    // It is assumed that the ISA interrupts will be identity-mapped into
    // the first IO APIC sources.

    // Enable hardware interrupts
    sti();
    log(LOG_LEVEL_DEBUG, "APIC: Hardware interrupt enabled\n");
}

#define PORT_PIC_MASTER_A 0x20
#define PORT_PIC_MASTER_B 0x21
#define PORT_PIC_SLAVE_A 0xa0
#define PORT_PIC_SLAVE_B 0xa1

static void disable_dual_8259a_pic(void) {
    // Initialize dual 8259a to be able to mask interrupts
    // If the 8259as are already initialized it just additionnaly write full 0
    // to OCW2 which shouldn't be a problem since we don't want to use 8259a.
    outb(PORT_PIC_MASTER_A, 0);
    outb(PORT_PIC_SLAVE_A, 0);
    outb(PORT_PIC_MASTER_B, 32);
    outb(PORT_PIC_SLAVE_B, 40);

    // Mask all interrupts
    outb(PORT_PIC_MASTER_B, 0xff);
    outb(PORT_PIC_SLAVE_B, 0xff);
}

static inline u32 ioapic_read_register(u32 offset) {
    *REG_IOREGSEL = offset;
    return *REG_IOWIN;
}

void ioapic_unmask_interrupt(u8 interrupt) {
    ioapic_set_io_redirection_table(
        interrupt, VECTOR_NUMBER_IOAPIC + interrupt, DELIVERY_MODE_FIXED, 0,
        POLARITY_ACTIVE_HIGH, TRIGGER_MODE_EDGE, 0, 0);
}

static void ioapic_set_io_redirection_table(u8 num, u8 vector, u8 delivery_mode,
                                            u8 destination_mode, u8 polarity,
                                            u8 trigger_mode, u8 mask,
                                            u8 destination) {
    *REG_IOREGSEL = IOREDTBL_LO(num);
    *REG_IOWIN = ((mask & 1U) << 16) | ((trigger_mode & 1U) << 15)
        | ((polarity & 1U) << 13) | ((destination_mode & 1U) << 11)
        | ((delivery_mode & 0b111U) << 8) | vector;

    *REG_IOREGSEL = IOREDTBL_HI(num);
    *REG_IOWIN = (u32)destination << (56 - 32);
}

void apic_eoi(void) {
    volatile u32 *isr = (volatile u32 *)(LAPIC_VIRT_ADDR + 0x100);
    for (u8 i = 0; i < 8; ++i) {
        if (*isr != 0) {
            for (u32 j = 0; j < 32; ++j) {
                if (((*isr >> j) & 1) == 1) {
                    kprintf("isr: %u\n", i * 32 + j);
                    break;
                }
            }
        }
        isr = (volatile u32 *)((u64)isr + 0x10);
    }

    // Just need a write to EOI
    *REG_EOI = 0;
}
