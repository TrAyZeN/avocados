ENTRY(_start);

SECTIONS {
    /*
     * The range _sboot-_eboot contains instructions and data for
     * boot that is discarded once pmm is initialized.
     */
    _sboot = .;
    .boot.text : {
        . = ALIGN(8);
        KEEP(*(.multiboot2));

        */boot.o(.text .text.*);
    }
    .boot.rodata : {
        */boot.o(.rodata .rodata.*);
    }
    .boot.data : {
        */boot.o(.data .data.*);
    }
    .boot.bss : {
        */boot.o(.bss .bss.*);
        */boot.o(COMMON);
    }
    _eboot = .;

    . = ALIGN(4096);
    _skern = .;

    .text : ALIGN(4096) {
        _stext = .;

        *(.text .text.*);
        KEEP(*(.test.*));

        _etext = .;
    }
    .rodata : ALIGN(4096) {
        _srodata = .;

        *(.rodata .rodata.*);

        /*
         * The .test_descriptors_align contains a single dummy test descriptor
         * to align _test_descriptors_start to the test descriptor array.
         */
        KEEP(*(.test_descriptors_align));
        _test_descriptors_start = .;
        KEEP(*(.test_descriptors));
        _test_descriptors_end = .;

        _erodata = .;
    }
    .eh_frame : ALIGN(4096) { 
        _eh_frame_start = .;
        KEEP(*(.eh_frame));
        _eh_frame_end = .;
    }

    .data : ALIGN(4096) {
        _sdata = .;
        *(.data .data.*);
        _edata = .;
    }
    .bss : ALIGN(4096) {
        _sbss = .;
        *(.bss .bss.*);
        *(COMMON);
        _ebss = .;
    }

    /* WARN: bss seems to overflow */
    /* TODO: Add a sort of canary to check overflow */

    _ekern = .;

    /DISCARD/ : { *(.note*) }
}
