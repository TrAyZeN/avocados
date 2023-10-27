#ifndef AVOCADOS_TEST_H_
#define AVOCADOS_TEST_H_

#include "attributes.h"

// Tests must panic when not passing
// Tests must left the global context as is

typedef void (*test_t)(void);

typedef struct {
    const char *name;
    test_t test;
} test_descriptor_t;

// WARN: The test_descriptor array only works if the linker packs the test
// descriptors. But is it a defined behaviour ?

#define DEFINE_TEST(test_name)                                                 \
    void test_name(void);                                                      \
                                                                               \
    static __attribute__((used, section(".test_descriptors")))                 \
    const test_descriptor_t test_descriptor = { .name = #test_name,            \
                                                .test = test_name };           \
                                                                               \
    __attribute__((section(".test." #test_name))) void test_name(void)

void run_tests(void);

#endif /* ! AVOCADOS_TEST_H_ */
