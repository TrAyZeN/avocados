#ifndef AVOCADOS_APIC_H_
#define AVOCADOS_APIC_H_

#include <stdbool.h>

#include "types.h"

// 32 to 64
#define VECTOR_NUMBER_IOAPIC 32
#define VECTOR_NUMBER_LINT0 64
#define VECTOR_NUMBER_LINT1 64
#define VECTOR_NUMBER_SPURIOUS_INT 255

void apic_init(u64 lapic_phys_addr, u64 ioapic_phys_addr, bool has_8259a);
void ioapic_unmask_interrupt(u8 interrupt);
void apic_eoi(void);

#endif /* ! AVOCADOS_APIC_H_ */
