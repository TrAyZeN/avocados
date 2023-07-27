#include <stdint.h>

#include "attributes.h"
#include "log.h"

#define DEF_ISR(VECTOR, HANDLER)                                               \
    __naked void isr_##VECTOR(void) {                                          \
        __asm__ volatile("push %rax\n"                                         \
                         "push %rbx\n"                                         \
                         "push %rcx\n"                                         \
                         "push %rdx\n"                                         \
                         "push %rsi\n"                                         \
                         "push %rdi\n"                                         \
                         "push %rbp\n"                                         \
                         "push %r8\n"                                          \
                         "push %r9\n"                                          \
                         "push %r10\n"                                         \
                         "push %r11\n"                                         \
                         "push %r12\n"                                         \
                         "push %r13\n"                                         \
                         "push %r14\n"                                         \
                         "push %r15\n"                                         \
                                                                               \
                         "call " #HANDLER "\n"                                 \
                                                                               \
                         "pop %r15\n"                                          \
                         "pop %r14\n"                                          \
                         "pop %r13\n"                                          \
                         "pop %r12\n"                                          \
                         "pop %r11\n"                                          \
                         "pop %r10\n"                                          \
                         "pop %r9\n"                                           \
                         "pop %r8\n"                                           \
                         "pop %rbp\n"                                          \
                         "pop %rdi\n"                                          \
                         "pop %rsi\n"                                          \
                         "pop %rdx\n"                                          \
                         "pop %rcx\n"                                          \
                         "pop %rbx\n"                                          \
                         "pop %rax\n"                                          \
                                                                               \
                         "iretq\n");                                           \
    }

struct context {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __packed;

DEF_ISR(0, divide_error);
__used static void divide_error(void) {
    log(LOG_LEVEL_DEBUG, "Interruption: Divide error\n");
}

DEF_ISR(1, debug_exception);
__used static void debug_exception(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Debug exception\n");
}

DEF_ISR(2, nmi);
__used static void nmi(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: NMI interrupt\n");
}

DEF_ISR(3, breakpoint);
__used static void breakpoint(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Breakpoint\n");
}

DEF_ISR(4, overflow);
__used static void overflow(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Overflow\n");
}

DEF_ISR(5, bound_range_exceeded);
__used static void bound_range_exceeded(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: BOUND range exceeded\n");
}

DEF_ISR(6, invalid_opcode);
__used static void invalid_opcode(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Invalid opcode\n");
}

DEF_ISR(7, device_not_available);
__used static void device_not_available(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG,
        "Interruption: Device not available (No math coprocessor)\n");
}

DEF_ISR(8, double_fault);
__used static void double_fault(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Double fault\n");
}

DEF_ISR(10, invalid_tss);
__used static void invalid_tss(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Invalid TSS\n");
}

DEF_ISR(11, segment_not_present);
__used static void segment_not_present(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Segment not present\n");
}

DEF_ISR(12, stack_segment_fault);
__used static void stack_segment_fault(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Stack-segment fault\n");
}

DEF_ISR(13, general_protection);
__used static void general_protection(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: General protection at %lx:%lx\n",
        ctx.cs, ctx.rip);
    __asm__ volatile("xchg %bx, %bx\n");
}

DEF_ISR(14, page_fault);
__used static void page_fault(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Page fault at %lx:%lx\n", ctx.cs,
        ctx.rip);
}

DEF_ISR(16, math_fault);
__used static void math_fault(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG,
        "Interruption: x87 FPU floating-point error (math fault)\n");
}

DEF_ISR(17, alignment_check);
__used static void alignment_check(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Alignment check\n");
}

DEF_ISR(18, machine_check);
__used static void machine_check(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Machine check\n");
}

DEF_ISR(19, simd_fpe);
__used static void simd_fpe(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: SIMD float-point exception\n");
}

DEF_ISR(21, control_protection);
__used static void control_protection(__unused struct context ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Control protection exception\n");
}
