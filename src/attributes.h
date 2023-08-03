#ifndef AVOCADOS_ATTRIBUTES_H_
#define AVOCADOS_ATTRIBUTES_H_

#define __packed __attribute__((packed))
#define __align(N) __attribute__((aligned(N)))
#define __naked __attribute__((naked))
#define __used __attribute__((used))
#define __unused __attribute__((unused))
#define __warn_unused_result __attribute__((warn_unused_result))
#define __format(ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)                      \
    __attribute__((format(ARCHETYPE, STRING_INDEX, FIRST_TO_CHECK)))

#endif /* ! AVOCADOS_ATTRIBUTES_H_ */
