#ifndef AVOCADOS_ATTRIBUTES_H_
#define AVOCADOS_ATTRIBUTES_H_

#define __packed __attribute__((packed))
#define __align(n) __attribute__((aligned(n)))
#define __naked __attribute__((naked))
#define __used __attribute__((used))
#define __unused __attribute__((unused))
#define __warn_unused_result __attribute__((warn_unused_result))
#define __format(archetype, string_index, first_to_check)                      \
    __attribute__((format(archetype, string_index, first_to_check)))

#endif /* ! AVOCADOS_ATTRIBUTES_H_ */
