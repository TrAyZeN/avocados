#ifndef AVOCADOS_FRAMEBUFFER_H_
#define AVOCADOS_FRAMEBUFFER_H_

#include "multiboot2.h"
#include "types.h"

// WARN: Only handles text mode

void fb_prepare(const struct multiboot_tag_framebuffer *framebuffer_tag);
u64 fb_init(void);

void fb_putchar(char c);
void fb_puts(const char *str);

#endif /* ! AVOCADOS_FRAMEBUFFER_H_ */
