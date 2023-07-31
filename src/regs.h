#ifndef AVOCADOS_REGISTERS_H_
#define AVOCADOS_REGISTERS_H_

// Control registers (See Vol. 3A 2.5)

// Enable/disable paging
#define CR0_PG (1 << 31)

// Enable/disable paging to produce addresses with more than 32 bits
#define CR4_PAE (1 << 5)
// 5-level paging/4-level paging
#define CR4_LA57 (1 << 12)

// Extended feature enable register (See Vol. 3A 2.2.1)
// Enables IA-32e mode operation
#define IA32_EFER_LME (1 << 8)
// Enables page access restriction by preventing instruction fetches from PAE
// pages with XD bit set.
#define IA32_EFER_NXE (1 << 11)

// MSR values
// See Vol. 4
#define MSR_IA32_EFER 0xc0000080

#endif /* ! AVOCADOS_REGISTERS_H_ */
