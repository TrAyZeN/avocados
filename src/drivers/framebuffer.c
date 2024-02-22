#include "framebuffer.h"
#include "libk/kassert.h"
#include "libk/log.h"
#include "mm/pmm.h" // Only there for PAGE_SIZE
#include "mm/vmm.h"
#include "utils.h"

// Virtual address of framebuffer mapping
#define FB_VIRT_ADDR 0x0000002000000000UL

#define FG_WHITE (15 << 8)

typedef struct {
    volatile u16 *buf;
    u32 width;
    u32 height;
    u32 x;
    u32 y;
} framebuffer_t;

static framebuffer_t framebuffer;
static u64 framebuffer_phys_addr;
static u32 framebuffer_width, framebuffer_height;

static void fb_clear_line(void);

void fb_prepare(const struct multiboot_tag_framebuffer *framebuffer_tag) {
    kassert(framebuffer_tag->common.framebuffer_type
            == MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT);

    framebuffer_phys_addr = framebuffer_tag->common.framebuffer_addr;
    framebuffer_width = framebuffer_tag->common.framebuffer_width;
    framebuffer_height = framebuffer_tag->common.framebuffer_height;
}

u64 fb_init(void) {
    u64 res = vmm_map_physical(
        FB_VIRT_ADDR, framebuffer_phys_addr,
        ALIGN_UP(framebuffer_width * framebuffer_height * 16, PAGE_SIZE), 0);
    if (res != 0) {
        return res;
    }

    framebuffer.buf = (volatile u16 *)FB_VIRT_ADDR;
    framebuffer.width = framebuffer_width;
    framebuffer.height = framebuffer_height;
    framebuffer.x = 0;
    framebuffer.y = 0;

    fb_clear_line();

    log(LOG_LEVEL_INFO, "Framebuffer initialized\n");

    return 0;
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
    u64 i = 0;
    while (str[i] != '\0') {
        fb_putchar(str[i]);
        i += 1;
    }
}

static void fb_clear_line(void) {
    for (u32 x = 0; x < framebuffer.width; ++x) {
        framebuffer.buf[x + framebuffer.y * framebuffer.width] = ' ' | FG_WHITE;
    }
}
