#include "framebuffer.h"
#include "log.h"

#define FG_WHITE (15 << 8)

struct framebuffer framebuffer;

static void fb_clear_line(void);

void fb_init(volatile uint16_t *buf, uint32_t width, uint32_t height) {
    framebuffer.buf = buf;
    framebuffer.width = width;
    framebuffer.height = height;
    framebuffer.x = 0;
    framebuffer.y = 0;

    fb_clear_line();

    log(LOG_LEVEL_INFO, "Framebuffer initialized\n");
}

void fb_putchar(char c) {
    if (c == '\n') {
        framebuffer.x = 0;
        framebuffer.y = (framebuffer.y + 1) % framebuffer.height;
        fb_clear_line();
    } else {
        framebuffer.buf[framebuffer.x + framebuffer.y * framebuffer.width] =
            c | FG_WHITE;

        if (framebuffer.x < framebuffer.width - 1) {
            framebuffer.x = framebuffer.x + 1;
        } else {
            framebuffer.x = 0;
            framebuffer.y = (framebuffer.y + 1) % framebuffer.height;
        }
    }
}

void fb_puts(const char *str) {
    uint64_t i = 0;
    while (str[i] != '\0') {
        fb_putchar(str[i]);
        i += 1;
    }
}

static void fb_clear_line(void) {
    for (uint32_t x = 0; x < framebuffer.width; ++x) {
        framebuffer.buf[x + framebuffer.y * framebuffer.width] = ' ' | FG_WHITE;
    }
}
