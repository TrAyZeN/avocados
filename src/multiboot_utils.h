#ifndef AVOCADOS_MULTIBOOT_UTILS_H_
#define AVOCADOS_MULTIBOOT_UTILS_H_

#include "multiboot2.h"

struct multiboot_tag *multiboot_find_tag(struct multiboot_tag *tag,
                                         multiboot_uint16_t type);
void multiboot_print_mmap(const struct multiboot_tag_mmap *mmap_tag);

#endif /* ! AVOCADOS_MULTIBOOT_UTILS_H_ */
