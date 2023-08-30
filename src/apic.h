#ifndef AVOCADOS_APIC_H_
#define AVOCADOS_APIC_H_

#include <stdbool.h>
#include <stdint.h>

#define VECTOR_NUMBER_IOAPIC 32
#define VECTOR_NUMBER_LINT0 33
#define VECTOR_NUMBER_LINT1 33
#define VECTOR_NUMBER_SPURIOUS_INT 255

void apic_init(uint64_t lapic_phys_addr, uint64_t ioapic_phys_addr,
               bool has_8259a);
void apic_eoi(void);

#endif /* ! AVOCADOS_APIC_H_ */
