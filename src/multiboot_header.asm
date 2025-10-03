; Set the magic number and flags for the Multiboot2 header.
; This header tells the bootloader (GRUB) that this is a Multiboot2-compliant kernel.

section .multiboot_header
header_start:
    ; Magic number - required by Multiboot2 spec
    dd 0xe85250d6
    ; Architecture - 0 for i386/x86_64
    dd 0
    ; Header length
    dd header_end - header_start
    ; Checksum - a value that when added to the other fields, results in zero.
    dd -(0xe85250d6 + 0 + (header_end - header_start))

    ; --- Multiboot2 Tags ---
    ; You can add tags here to request features from the bootloader,
    ; like a framebuffer or memory map. For now, we'll just have the end tag.

    ; End tag - marks the end of the tags list.
    dw 0 ; type
    dw 0 ; flags
    dd 8 ; size
header_end:
