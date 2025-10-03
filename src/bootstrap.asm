section .text
bits 32 ; GRUB starts us in 32-bit protected mode

; Aligns the machine code to a 4-byte boundary
align 4
; Makes the `start` symbol visible to the linker
global start

extern kmain ; kmain is defined in our C code

start:
    ; Disable interrupts until the IDT is set up
    cli

    ; Set up the stack. We'll allocate a 16KB stack.
    ; We need a stack before we can call C functions.
    mov esp, stack_top

    ; Push the Multiboot2 magic number and the address of the info structure
    ; onto the stack. These are the arguments for our kmain function.
    push eax ; Multiboot2 magic number (from GRUB)
    push ebx ; Address of Multiboot2 info structure

    ; Call the C kernel's main function
    call kmain

    ; If kmain ever returns (it shouldn't), halt the CPU.
    cli
.hang:
    hlt
    jmp .hang

section .bss
; Reserve 16KB for the stack
align 16
stack_bottom:
    resb 16384 ; 16 * 1024
stack_top:
