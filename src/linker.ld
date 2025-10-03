/*
 * A simple linker script for our kernel.
 */

ENTRY(start) /* The entry point of our kernel is the 'start' label */

SECTIONS {
    /* The kernel will be loaded at 1MB */
    . = 1M;

    /* The first section is the .multiboot_header, which must be at the beginning */
    .text : {
        *(.multiboot_header)
        *(.text)
    }

    /* Read-only data */
    .rodata : {
        *(.rodata)
    }

    /* Read-write data */
    .data : {
        *(.data)
    }

    /* Uninitialized data */
    .bss : {
        *(.bss)
    }

    end = .;
}
