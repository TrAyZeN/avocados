#include <stddef.h>
#include <stdint.h>

#include "libk/kprintf.h"
#include "test.h"

// This dummy test descriptor is put in its own section, used to align
// _test_descriptors_start on the test descriptor array.
// WARN: It currently works but is it a defined behaviour ?
static __attribute__((used, section(".test_descriptors_align")))
const test_descriptor_t _test_descriptors_align = { .name = NULL,
                                                    .test = NULL };

extern const test_descriptor_t _test_descriptors_start, _test_descriptors_end;

void run_tests(void) {
    for (const test_descriptor_t *test_desc = &_test_descriptors_start;
         test_desc < &_test_descriptors_end; ++test_desc) {
        kprintf("Running test: %s\n", test_desc->name);
        test_desc->test();
    }

    puts("All tests passed\n");
}
