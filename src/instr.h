#ifndef AVOCADOS_INSTR_H_
#define AVOCADOS_INSTR_H_

#include "types.h"

static inline void sti(void) {
    __asm__ volatile("sti");
}

static inline void cli(void) {
    __asm__ volatile("cli");
}

static inline void hlt(void) {
    __asm__ volatile("hlt");
}

static inline void pause(void) {
    __asm__ volatile("pause");
}

static inline void outb(u16 port, u8 val) {
    __asm__ volatile("outb %0, %1" : /* No output */ : "a"(val), "d"(port));
}

static inline u8 inb(u16 port) {
    u8 res;

    __asm__ volatile("inb %1, %0" : "=&a"(res) : "d"(port));

    return res;
}

static inline void outw(u16 port, u16 val) {
    __asm__ volatile("outw %0, %1" : /* No output */ : "a"(val), "d"(port));
}

static inline u16 inw(u16 port) {
    u16 res;

    __asm__ volatile("inw %1, %0" : "=&a"(res) : "d"(port));

    return res;
}

static inline void outd(u16 port, u32 val) {
    __asm__ volatile("out %0, %1" : /* No output */ : "a"(val), "d"(port));
}

static inline u32 ind(u16 port) {
    u32 res;

    __asm__ volatile("in %1, %0" : "=&a"(res) : "d"(port));

    return res;
}

static inline u64 rdmsr(u32 msr) {
    u32 res_lo, res_hi;

    __asm__ volatile("rdmsr" : "=&d"(res_hi), "=&a"(res_lo) : "c"(msr));

    return ((u64)res_hi << 32) | res_lo;
}

#endif /* ! AVOCADOS_INSTR_H_ */
