/*
 * This file contains multiboot2 header and initialization code ran just after
 * the bootloader (as multiboot does not support booting in long mode).
 *
 * The initialization code performs the following steps:
 * - Load boot GDT
 * - Setup basic page tables
 * - Long mode initialization
 * - Load kernel GDT
 * - Setup kernel stack
 * - Jump to kmain
 */

#define ASM_FILE
#include "multiboot2.h"
#include "arch/regs.h"
#include "arch/gdt.h"
#include "utils.h"
#include "avocados.h"

// Multiboot2 specification:
// https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html

    .section .multiboot2
    .align MULTIBOOT_HEADER_ALIGN
    .global multiboot_header
multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC
    .long MULTIBOOT_ARCHITECTURE_I386
    .long multiboot_header_end - multiboot_header
    .long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))

address_tag_start:
    .align MULTIBOOT_TAG_ALIGN
    .short MULTIBOOT_HEADER_TAG_ADDRESS
    .short MULTIBOOT_HEADER_TAG_OPTIONAL
    .long address_tag_end - address_tag_start
    .long multiboot_header
    .long _sboot
    .long _edata
    .long _ebss
address_tag_end:

entry_address_tag_start:
    .align MULTIBOOT_TAG_ALIGN
    .short MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS
    .short MULTIBOOT_HEADER_TAG_OPTIONAL
    .long entry_address_tag_end - entry_address_tag_start
    .long _start
entry_address_tag_end:

    .align MULTIBOOT_TAG_ALIGN
    .short MULTIBOOT_HEADER_TAG_END
    .short 0
    .long multiboot_header_end - entry_address_tag_end
multiboot_header_end:

.extern kernel_stack

.extern gdt

.extern pml4
.extern pdpt
.extern pdt
.extern pt

    .data
    .align GDT_ALIGN
boot_gdt:
    .8byte 0
#define BOOT_GDT_IDX_CODE 1
    .8byte BYTES_GDT_SEG_DESCRIPTOR(
        0,
        0xffffff,
        SEG_TYPE_RE,
        SEG_DPL_RING_0,
        0,
        1,
        SEG_GRAN_4KIB
    )
#define BOOT_GDT_IDX_DATA 2
    .8byte BYTES_GDT_SEG_DESCRIPTOR(
        0,
        0xffffff,
        SEG_TYPE_RW,
        SEG_DPL_RING_0,
        0,
        1,
        SEG_GRAN_4KIB
    )
boot_gdt_end:

gdt_descriptor:
    .short boot_gdt_end - boot_gdt - 1
    .long boot_gdt
    .long 0 // For 64 bit register

    .code32

    .section .text
    .global _start
    .type _start, @function
_start:
    // Interrupt and paging are already disabled
    // https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html#I386-machine-state

    // Save multiboot information for kmain
    mov %eax, %edi // Multiboot magic value
    mov %ebx, %esi // Multiboot information structure

    // Load temporary boot GDT because multiboot GDTR may be invalid
    lgdt gdt_descriptor

    // Segment selector (see Vol. 3A 3.4.2)
    mov $(SEGMENT_SELECTOR(BOOT_GDT_IDX_DATA, SEGMENT_SELECTOR_GDT, 0)), %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    // Far jump to reload code segment
    ljmp $(SEGMENT_SELECTOR(BOOT_GDT_IDX_CODE, SEGMENT_SELECTOR_GDT, 0)), $long_mode_init

    .local long_mode_init
long_mode_init:
    // Page table initialization (see Vol. 3A 4.5.4)

    // Setup only first pml4 entry for boot. Thus kernel code should fit within
    // the first 512GiB.
    lea pdpt, %eax
    and $0xfffff000, %eax
    or $((1 << 0) | (1 << 1)), %eax
    mov %eax, pml4
    // pdpt should be placed in the first 32 bit of the physical address space
    mov $0, %eax
    mov %eax, pml4 + 4

    // Setup recursive paging
    lea pml4, %eax
    and $0xfffff000, %eax
    or $((1 << 0) | (1 << 1)), %eax
    mov %eax, pml4 + (511 * 8)
    mov $0, %eax
    mov %eax, pml4 + (511 * 8 + 4)

    // Setup only first pdpt entry for boot. Thus kernel code should fit within
    // the first 1GiB.
    lea pdt, %eax
    and $0xfffff000, %eax
    or $((1 << 0) | (1 << 1)), %eax
    mov %eax, pdpt
    // pdt should be placed in the first 32 bit address space of the physical
    // address space
    mov $0, %eax
    mov %eax, pdpt + 4

    // Setup only first pd entry. Thus kernel code should fit within the first
    // 2MiB.
    lea pt, %eax
    and $0xfffff000, %eax
    or $((1 << 0) | (1 << 1)), %eax
    mov %eax, pdt
    // pt should be placed in the first 32 bit address space of the physical
    // address space
    mov $0, %eax
    mov %eax, pdt + 4

    // Identity map page frames of boot and kernel ranges.
    // We are assuming that they in a single page table.
    mov $0, %ecx
    lea pt, %ebx
    .local identity_map_boot_kernel_pages_loop
identity_map_boot_kernel_pages_loop:
    mov %ecx, %eax
    shl $12, %eax
    or $((1 << 0) | (1 << 1)), %eax
    mov %eax, (%ebx)
    add $4, %ebx

    movl $0, (%ebx)
    add $4, %ebx
    add $1, %ecx

    // Number of pages for boot and kernel range
    mov $_ekern, %eax
    add $(4096 - 1), %eax
    shr $12, %eax
    cmp %eax, %ecx
    jl identity_map_boot_kernel_pages_loop

    // Identity map multiboot information structure. No loop on the size for
    // simplicity, we are assuming that boot info is on a single page frame.
    mov %esi, %eax
    shr $12, %eax // Page index
    shl $3, %eax // Page table offset (a page table entry is 8 (1 << 3) bytes)
    lea pt, %ebx
    add %eax, %ebx

    mov %esi, %eax
    and $0xfffff000, %eax
    or $((1 << 0) | (1 << 1)), %eax
    mov %eax, (%ebx)
    add $4, %ebx
    // Multiboot boot information should be placed in the first 32 bit address
    // space of the physical address space
    movl $(1 << (63 - 32)), (%ebx)

    // IA-32e mode initialization (see Vol 3A 10.8.5)

    // Enable Physical-Address Extensions and 4 level paging
    mov %cr4, %eax
    or $CR4_PAE, %eax
    and $(~CR4_LA57), %eax
    mov %eax, %cr4

    // Load the address of the PML4
    // WARN: CR3.PCD and CR3.PWT are wiped
    lea pml4, %eax
    mov %eax, %cr3

    // TODO: CPUID
    // Enable IA-32e mode (see Vol. 3A 2.2.1)
    // Also enables page access restrictions
    mov $MSR_IA32_EFER, %ecx
    rdmsr
    or $(IA32_EFER_LME | IA32_EFER_NXE), %eax
    wrmsr

    // Enable paging
    mov %cr0, %eax
    or $CR0_PG, %eax
    mov %eax, %cr0

    // Load "real" GDT with 64-bit code segment
    movw $(GDT_NUM_ENTRIES * GDT_ENTRY_SIZE - 1), gdt_descriptor
    lea gdt, %eax
    movl %eax, gdt_descriptor + 2
    movl $0, gdt_descriptor + 6
    lgdt gdt_descriptor

    movw $(SEGMENT_SELECTOR(GDT_IDX_DATA, SEGMENT_SELECTOR_GDT, 0)), %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    lea (kernel_stack + KERNEL_STACK_SIZE - 1), %esp
    and $0xfffffff0, %esp
    push $(SEGMENT_SELECTOR(GDT_IDX_CODE, SEGMENT_SELECTOR_GDT, 0))
    push $load_gdt_segments_caller_ret
    lret

    .code64
    .local load_gdt_segments_caller_ret
load_gdt_segments_caller_ret:
    lea (kernel_stack + KERNEL_STACK_SIZE - 1), %rsp
    and $0xfffffffffffffff0, %rsp
    jmp kmain

    // We should not return from kmain
    cli
    .local _dead_loop
_dead_loop:
    hlt
    jmp _dead_loop
