#include "apic.h"
#include "hpet.h"
#include "kassert.h"
#include "kprintf.h"
#include "panic.h"
#include "utils.h"
#include "vmm.h"

#define HPET_VIRT_ADDR 0x0000001100002000UL

#define REG_GENERAL_CAPABILITIES_AND_ID                                        \
    ((volatile uint64_t *)(HPET_VIRT_ADDR + 0x00))
// Software should not modify the value in these bits until they are define.
// This is done by doing a "read-modify-write" to this register. (See HPET
// specification 1.0a, 2.3.5)
#define REG_GENERAL_CONFIG ((volatile uint64_t *)(HPET_VIRT_ADDR + 0x10))
#define REG_MAIN_COUNTER_VALUE ((volatile uint64_t *)(HPET_VIRT_ADDR + 0xf0))
#define REG_TIMER_CONFIG_AND_CAPABILITY(TIMER_NUM)                             \
    ((volatile uint64_t *)(HPET_VIRT_ADDR + 0x100 + (TIMER_NUM)*0x20))
#define REG_TIMER_COMPARATOR(TIMER_NUM)                                        \
    ((volatile uint64_t *)(HPET_VIRT_ADDR + 0x108 + (TIMER_NUM)*0x20))

#define GET_NUM_TIM_CAP(VAL) (BIT_RANGE(VAL, 8, 12))
#define GET_LEG_RT_CAP(VAL) (BIT_RANGE(VAL, 15, 15))

#define ENABLE_CNF_BIT 0
#define LEG_RT_CNF_BIT 1

#define INT_TYPE_CNF_EDGE 0
#define INT_ENB_CNF_ENABLE 1
#define TYPE_CNF_PERIODIC 1
#define VAL_SET_CNF_ENABLE 1

#define TIMER_CONFIG_AND_CAPABILITY(INT_TYPE_CNF, INT_ENB_CNF, TYPE_CNF,       \
                                    VAL_SET_CNF, MODE32_CNF, INT_ROUTE_CNF,    \
                                    FSB_EN_CNF, INT_ROUTE_CAP)                 \
    (BIT_BLOCK(INT_TYPE_CNF, 1, 1) | BIT_BLOCK(INT_ENB_CNF, 2, 2)              \
     | BIT_BLOCK(TYPE_CNF, 3, 3) | BIT_BLOCK(VAL_SET_CNF, 6, 6)                \
     | BIT_BLOCK(MODE32_CNF, 8, 8) | BIT_BLOCK(INT_ROUTE_CNF, 9, 13)           \
     | BIT_BLOCK(FSB_EN_CNF, 14, 14) | BIT_BLOCK(INT_ROUTE_CAP, 32, 64))

static void hpet_set_general_config(uint8_t overall_enable,
                                    uint8_t legacy_replacement_route);
static void hpet_print_general_capabilities_and_id(void);

void hpet_init(uint64_t hpet_phys_addr) {
    uint64_t res = vmm_map_physical(HPET_VIRT_ADDR, hpet_phys_addr, 4096, 0);
    if (res == VMM_ALLOC_ERROR) {
        kpanic("Failed to map HPET registers\n");
    }

    hpet_print_general_capabilities_and_id();
    uint64_t general_capabilities_and_id = *REG_GENERAL_CAPABILITIES_AND_ID;
    kassert(GET_LEG_RT_CAP(general_capabilities_and_id) == 1);
    kassert(GET_NUM_TIM_CAP(general_capabilities_and_id)
            >= 1); // at least 2 timers

    // Halt main counter
    hpet_set_general_config(0, 0);

    // Reset main counter
    *REG_MAIN_COUNTER_VALUE = 0;

    kprintf("Timer 0 configuration: 0x%lx\n",
            *REG_TIMER_CONFIG_AND_CAPABILITY(0));

    // Note for timer interrupt route:
    // If the LegacyReplacement Route bit is set, then Timers 0 and 1 will
    // have a different routing, and this bit field has no effect for those two
    // timers
    *REG_TIMER_CONFIG_AND_CAPABILITY(0) = TIMER_CONFIG_AND_CAPABILITY(
        INT_TYPE_CNF_EDGE, INT_ENB_CNF_ENABLE, TYPE_CNF_PERIODIC,
        VAL_SET_CNF_ENABLE, 0, 0, 0, 0);
    *REG_TIMER_COMPARATOR(0) = 100000000;

    kprintf("Timer 0 configuration: 0x%lx\n",
            *REG_TIMER_CONFIG_AND_CAPABILITY(0));

    // Enable counters and support legacy replacement route
    hpet_set_general_config(1, 1);

    // Enable timer 0 interrupt (as legacy replacement route is enabled, timer 0
    // is routed to irq2 of the ioapic)
    ioapic_unmask_interrupt(2);
}

static void hpet_set_general_config(uint8_t overall_enable,
                                    uint8_t legacy_replacement_route) {
    // Only one bit values
    kassert((overall_enable & 1) == overall_enable);
    kassert((legacy_replacement_route & 1) == legacy_replacement_route);

    uint64_t general_configuration = *REG_GENERAL_CONFIG;
    general_configuration |= ((overall_enable & 1U) << ENABLE_CNF_BIT)
        | ((legacy_replacement_route & 1U) << LEG_RT_CNF_BIT);
    // xor 1 is used to invert the bit value for the and mask
    general_configuration &=
        ~((((overall_enable & 1U) ^ 1) << ENABLE_CNF_BIT)
          | (((legacy_replacement_route & 1U) ^ 1) << LEG_RT_CNF_BIT));
    *REG_GENERAL_CONFIG = general_configuration;
}

static void hpet_print_general_capabilities_and_id(void) {
    uint64_t general_capabilities_and_id = *REG_GENERAL_CAPABILITIES_AND_ID;

    kprintf("Revision id: %lu\n", general_capabilities_and_id & 0xff);
    kprintf("Number of timers: %lu\n",
            BIT_RANGE(general_capabilities_and_id, 8, 12));
    kprintf("Counter size: %lu\n",
            BIT_RANGE(general_capabilities_and_id, 13, 13));
    kprintf("Legacy replacement route capable: %lu\n",
            BIT_RANGE(general_capabilities_and_id, 15, 15));
    kprintf("Vendor id: %lu\n", BIT_RANGE(general_capabilities_and_id, 16, 31));
    kprintf("Main counter tick period: %lu (in femptoseconds)\n",
            general_capabilities_and_id >> 32);
}
