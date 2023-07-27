#ifndef AVOCADOS_INSTR_H_
#define AVOCADOS_INSTR_H_

#include <stdint.h>

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

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : /* No output */ : "a"(val), "d"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t res;

    __asm__ volatile("inb %1, %0" : "=&a"(res) : "d"(port));

    return res;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : /* No output */ : "a"(val), "d"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t res;

    __asm__ volatile("inw %1, %0" : "=&a"(res) : "d"(port));

    return res;
}

#endif /* ! AVOCADOS_INSTR_H_ */
