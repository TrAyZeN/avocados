CC ?= gcc
# AS ?= gcc

CPPFLAGS += -MMD -Isrc
CFLAGS += -std=c11 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Wundef \
	 -Wconversion -Wsign-conversion -Wformat=2 -O0 -fno-builtin -ffreestanding \
	 -funsigned-char -fno-pie -fno-common -m64 -march=x86-64 \
	 -ffunction-sections -fdata-sections -fno-stack-protector -mno-red-zone \
	 -funwind-tables -fsanitize=undefined
LDFLAGS += -static -nostartfiles -nostdlib -mno-red-zone -lgcc \
	-Wl,--build-id=none,--gc-sections,--print-gc-sections
ASFLAGS +=

ifeq ($(CC),clang)
	CFLAGS += -Weverything
endif

ifeq ($(DEBUG),1)
	CPPFLAGS += -DDEBUG
	CFLAGS += -g
	CFLAGS += -fno-strict-aliasing
endif

ifeq ($(VERBOSE_BUILD),1)
	LDFLAGS += -Wl,--print-map
endif

BUILD_DIR ?= build
OBJS_DIR := $(BUILD_DIR)/objs

ISO := avocados.iso
BIN := avocados.bin
C_SRCS := \
	 src/kmain.c \
	 src/gdt.c \
	 src/kprintf.c \
	 src/framebuffer.c \
	 src/idt.c \
	 src/isr.c \
	 src/pmm.c \
	 src/multiboot_utils.c \
	 src/panic.c \
	 src/paging.c \
	 src/backtrace.c \
	 src/ubsan.c \
	 src/vmm.c \
	 src/serial.c \
	 src/test.c \
	 src/log.c \
	 src/acpi.c \
	 src/utils.c
S_SRCS := src/boot.S
OBJS := $(C_SRCS:%.c=$(OBJS_DIR)/%.o) $(S_SRCS:%.S=$(OBJS_DIR)/%.o)
DEPS := $(OBJS:%.o=%.d)

SRC_SUBDIRS := $(dir $(C_SRCS)) $(dir $(S_SRCS))
DIRS := $(SRC_SUBDIRS:%=$(OBJS_DIR)/%)
DIRS += $(BUILD_DIR)/iso/boot/grub/

.PHONY: all clean c fmt

all: $(ISO)

run r: $(ISO)
	# qemu-system-x86_64 -cdrom $< -serial stdio -m 1024M
	qemu-system-x86_64 -cdrom $< -m 1024M -nographic -serial mon:stdio

run_bochs rb: $(ISO)
	bochs -q -rc .bochs_commands

$(ISO): $(DIRS) $(BUILD_DIR)/$(BIN) grub.cfg
	cp grub.cfg $(BUILD_DIR)/iso/boot/grub
	cp $(BUILD_DIR)/$(BIN) $(BUILD_DIR)/iso/boot
	grub-mkrescue -o $@ $(BUILD_DIR)/iso

%.bin: %.elf
	objcopy -O binary -S $< $@

$(BUILD_DIR)/avocados.elf: $(OBJS) link.ld
	$(CC) $(filter %.o,$^) -o $@ $(LDFLAGS) -T link.ld

$(OBJS_DIR)/%.o: %.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

$(OBJS_DIR)/%.o: %.S
	$(COMPILE.S) $(filter %.S,$^) -o $@

%/:
	mkdir -p $@

clean c:
	$(RM) -r $(BUILD_DIR)

fmt:
	clang-format --style=file -i src/**.[ch]

-include $(DEPS)
