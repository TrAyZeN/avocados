#include <stddef.h>
#include <stdint.h>

#include "kprintf.h"
#include "test.h"

// This dummy test descriptor is put in its own section, used to align
// _test_descriptors_start on the test descriptor array.
// WARN: It currently works but is it a defined behaviour ?
static __attribute__((used, section(".test_descriptors_align")))
const struct test_descriptor _test_descriptors_align = { .name = NULL,
                                                         .test = NULL };

extern const struct test_descriptor _test_descriptors_start,
    _test_descriptors_end;

void run_tests(void) {
    for (const struct test_descriptor *test_desc = &_test_descriptors_start;
         test_desc < &_test_descriptors_end; ++test_desc) {
        kprintf("Running test: %s\n", test_desc->name);
        test_desc->test();
    }

    puts("All tests passed\n");
}
