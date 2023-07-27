#ifndef AVOCADOS_FRAMEBUFFER_H_
#define AVOCADOS_FRAMEBUFFER_H_

#include <stdint.h>

// WARN: Only handles text mode

struct framebuffer {
    volatile uint16_t *buf;
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;
};

extern struct framebuffer framebuffer;

void fb_init(volatile uint16_t *buf, uint32_t width, uint32_t height);

void fb_putchar(char c);
void fb_puts(const char *str);

#endif /* ! AVOCADOS_FRAMEBUFFER_H_ */
