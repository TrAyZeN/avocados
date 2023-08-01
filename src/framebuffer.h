#ifndef AVOCADOS_FRAMEBUFFER_H_
#define AVOCADOS_FRAMEBUFFER_H_

#include <stdint.h>

#include "multiboot2.h"

// WARN: Only handles text mode

struct framebuffer {
    volatile uint16_t *buf;
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;
};

void fb_prepare(const struct multiboot_tag_framebuffer *framebuffer_tag);
uint64_t fb_init(void);

void fb_putchar(char c);
void fb_puts(const char *str);

#endif /* ! AVOCADOS_FRAMEBUFFER_H_ */
