CC ?= gcc
# AS ?= gcc

CPPFLAGS += -MMD -Isrc
CFLAGS += -std=gnu17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Wundef \
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
	 src/arch/gdt.c \
	 src/libk/kprintf.c \
	 src/drivers/framebuffer.c \
	 src/arch/idt.c \
	 src/arch/isr.c \
	 src/mm/pmm.c \
	 src/multiboot_utils.c \
	 src/libk/panic.c \
	 src/arch/paging.c \
	 src/backtrace.c \
	 src/tools/ubsan.c \
	 src/mm/vmm.c \
	 src/drivers/serial.c \
	 src/tools/test.c \
	 src/libk/log.c \
	 src/drivers/acpi.c \
	 src/drivers/apic.c \
	 src/drivers/hpet.c \
	 src/drivers/pci.c \
	 src/libk/string.c \
	 src/libk/mem.c \
	 src/libk/bitmap.c
S_SRCS := src/arch/boot.S
OBJS := $(C_SRCS:%.c=$(OBJS_DIR)/%.o) $(S_SRCS:%.S=$(OBJS_DIR)/%.o)
DEPS := $(OBJS:%.o=%.d)

SRC_SUBDIRS := $(dir $(C_SRCS)) $(dir $(S_SRCS))
DIRS := $(SRC_SUBDIRS:%=$(OBJS_DIR)/%)
DIRS += $(BUILD_DIR)/iso/boot/grub/

.PHONY: all clean c fmt

all: $(ISO)

run r: $(ISO)
	qemu-img create -f qcow2 disk.img 2G
	qemu-system-x86_64 \
		-m 1024M \
		-smp cores=2,threads=2 \
		-cdrom $< \
		-drive id=disk,file=disk.img,if=none \
		-device ahci,id=ahci \
		-device ide-hd,drive=disk,bus=ahci.0 \
		-serial stdio
	# qemu-system-x86_64 -m 1024M -smp cores=2,threads=2 -cdrom $< \
		# -hda drive.img -nographic -serial mon:stdio

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
	clang-format --style=file -i src/**.{c,h}

-include $(DEPS)
