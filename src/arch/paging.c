#include "paging.h"

// Uninitialized static storage duration objects are initialized to zero
// (see C11 standard, clause 6.7.9.10)

// WARN: We need these arrays to be initialized to zero so that present bit is
// 0.
pml4_t pml4;
pdpt_t pdpt;
pdt_t pdt;
pt_t pt;
