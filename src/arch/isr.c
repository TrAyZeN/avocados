#include "attributes.h"
#include "drivers/apic.h"
#include "libk/log.h"
#include "types.h"

#define DEF_ISR(VECTOR, HANDLER) _DEF_ISR(VECTOR, HANDLER)

#define _DEF_ISR(VECTOR, HANDLER)                                              \
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

typedef struct {
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rbp;
    u64 rdi;
    u64 rsi;
    u64 rdx;
    u64 rcx;
    u64 rbx;
    u64 rax;
    u64 error_code;
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} __packed context_t;

DEF_ISR(0, divide_error);
__used static void divide_error(void) {
    log(LOG_LEVEL_DEBUG, "Interruption: Divide error\n");
}

DEF_ISR(1, debug_exception);
__used static void debug_exception(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Debug exception\n");
}

DEF_ISR(2, nmi);
__used static void nmi(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: NMI interrupt\n");
}

DEF_ISR(3, breakpoint);
__used static void breakpoint(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Breakpoint\n");
}

DEF_ISR(4, overflow);
__used static void overflow(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Overflow\n");
}

DEF_ISR(5, bound_range_exceeded);
__used static void bound_range_exceeded(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: BOUND range exceeded\n");
}

DEF_ISR(6, invalid_opcode);
__used static void invalid_opcode(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Invalid opcode\n");
}

DEF_ISR(7, device_not_available);
__used static void device_not_available(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG,
        "Interruption: Device not available (No math coprocessor)\n");
}

DEF_ISR(8, double_fault);
__used static void double_fault(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Double fault\n");
}

DEF_ISR(10, invalid_tss);
__used static void invalid_tss(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Invalid TSS\n");
}

DEF_ISR(11, segment_not_present);
__used static void segment_not_present(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Segment not present\n");
}

DEF_ISR(12, stack_segment_fault);
__used static void stack_segment_fault(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Stack-segment fault\n");
}

DEF_ISR(13, general_protection);
__used static void general_protection(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: General protection at %016lx:%016lx\n",
        ctx.cs, ctx.rip);
    __asm__ volatile("xchg %bx, %bx\n");
}

DEF_ISR(14, page_fault);
__used static void page_fault(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Page fault at %016lx:%016lx\n", ctx.cs,
        ctx.rip);
}

DEF_ISR(16, math_fault);
__used static void math_fault(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG,
        "Interruption: x87 FPU floating-point error (math fault)\n");
}

DEF_ISR(17, alignment_check);
__used static void alignment_check(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Alignment check\n");
}

DEF_ISR(18, machine_check);
__used static void machine_check(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Machine check\n");
}

DEF_ISR(19, simd_fpe);
__used static void simd_fpe(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: SIMD float-point exception\n");
}

DEF_ISR(21, control_protection);
__used static void control_protection(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: Control protection exception\n");
}

DEF_ISR(32, io_external_interrupt_0);
__used static void io_external_interrupt_0(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 32)\n");
    apic_eoi();
}

DEF_ISR(33, io_external_interrupt_1);
__used static void io_external_interrupt_1(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 33)\n");
    apic_eoi();
}

DEF_ISR(34, io_external_interrupt_2);
__used static void io_external_interrupt_2(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 34)\n");
    apic_eoi();
}

DEF_ISR(35, io_external_interrupt_3);
__used static void io_external_interrupt_3(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 35)\n");
    apic_eoi();
}

DEF_ISR(36, io_external_interrupt_4);
__used static void io_external_interrupt_4(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 36)\n");
    apic_eoi();
}

DEF_ISR(37, io_external_interrupt_5);
__used static void io_external_interrupt_5(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 37)\n");
    apic_eoi();
}

DEF_ISR(38, io_external_interrupt_6);
__used static void io_external_interrupt_6(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 38)\n");
    apic_eoi();
}

DEF_ISR(39, io_external_interrupt_7);
__used static void io_external_interrupt_7(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 39)\n");
    apic_eoi();
}

DEF_ISR(40, io_external_interrupt_8);
__used static void io_external_interrupt_8(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 40)\n");
    apic_eoi();
}

DEF_ISR(41, io_external_interrupt_9);
__used static void io_external_interrupt_9(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 41)\n");
    apic_eoi();
}

DEF_ISR(42, io_external_interrupt_10);
__used static void io_external_interrupt_10(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 42)\n");
    apic_eoi();
}

DEF_ISR(43, io_external_interrupt_11);
__used static void io_external_interrupt_11(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 43)\n");
    apic_eoi();
}

DEF_ISR(44, io_external_interrupt_12);
__used static void io_external_interrupt_12(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 44)\n");
    apic_eoi();
}

DEF_ISR(45, io_external_interrupt_13);
__used static void io_external_interrupt_13(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 45)\n");
    apic_eoi();
}

DEF_ISR(46, io_external_interrupt_14);
__used static void io_external_interrupt_14(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 46)\n");
    apic_eoi();
}

DEF_ISR(47, io_external_interrupt_15);
__used static void io_external_interrupt_15(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 47)\n");
    apic_eoi();
}

DEF_ISR(48, io_external_interrupt_16);
__used static void io_external_interrupt_16(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 48)\n");
    apic_eoi();
}

DEF_ISR(49, io_external_interrupt_17);
__used static void io_external_interrupt_17(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 49)\n");
    apic_eoi();
}

DEF_ISR(50, io_external_interrupt_18);
__used static void io_external_interrupt_18(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 50)\n");
    apic_eoi();
}

DEF_ISR(51, io_external_interrupt_19);
__used static void io_external_interrupt_19(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 51)\n");
    apic_eoi();
}

DEF_ISR(52, io_external_interrupt_20);
__used static void io_external_interrupt_20(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 52)\n");
    apic_eoi();
}

DEF_ISR(53, io_external_interrupt_21);
__used static void io_external_interrupt_21(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 53)\n");
    apic_eoi();
}

DEF_ISR(54, io_external_interrupt_22);
__used static void io_external_interrupt_22(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 54)\n");
    apic_eoi();
}

DEF_ISR(55, io_external_interrupt_23);
__used static void io_external_interrupt_23(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 55)\n");
    apic_eoi();
}

DEF_ISR(56, io_external_interrupt_24);
__used static void io_external_interrupt_24(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 56)\n");
    apic_eoi();
}

DEF_ISR(57, io_external_interrupt_25);
__used static void io_external_interrupt_25(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 57)\n");
    apic_eoi();
}

DEF_ISR(58, io_external_interrupt_26);
__used static void io_external_interrupt_26(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 58)\n");
    apic_eoi();
}

DEF_ISR(59, io_external_interrupt_27);
__used static void io_external_interrupt_27(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 59)\n");
    apic_eoi();
}

DEF_ISR(60, io_external_interrupt_28);
__used static void io_external_interrupt_28(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 60)\n");
    apic_eoi();
}

DEF_ISR(61, io_external_interrupt_29);
__used static void io_external_interrupt_29(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 61)\n");
    apic_eoi();
}

DEF_ISR(62, io_external_interrupt_30);
__used static void io_external_interrupt_30(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 62)\n");
    apic_eoi();
}

DEF_ISR(63, io_external_interrupt_31);
__used static void io_external_interrupt_31(__unused context_t ctx) {
    log(LOG_LEVEL_DEBUG, "Interruption: External interrupt (int 63)\n");
    apic_eoi();
}
