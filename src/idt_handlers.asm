; Common stub for all ISRs
; This stub saves CPU state, calls the C handler, and restores state
global isr_common_stub
extern isr_handler

isr_common_stub:
    ; Save all registers
    pusha                   ; Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    push ds                 ; Push data segment
    push es
    push fs
    push gs
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Push ESP (pointer to registers structure) as argument
    push esp
    
    ; Call the C interrupt handler
    call isr_handler
    
    ; Pop the stack pointer argument
    add esp, 4
    
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restore general purpose registers
    popa
    
    ; Clean up error code and interrupt number
    add esp, 8
    
    ; Return from interrupt
    iret

; ISRs for exceptions (0-31)
; These do not push an error code
%macro NO_ERRCODE_ISR 1
    global isr%1
    isr%1:
        cli                 ; Disable interrupts
        push byte 0         ; Push dummy error code
        push byte %1        ; Push interrupt number
        jmp isr_common_stub
%endmacro

; These ISRs push an error code automatically
%macro ERRCODE_ISR 1
    global isr%1
    isr%1:
        cli                 ; Disable interrupts
        ; Error code is already on stack
        push byte %1        ; Push interrupt number
        jmp isr_common_stub
%endmacro

; Exception ISRs (0-31)
NO_ERRCODE_ISR 0    ; Division by zero
NO_ERRCODE_ISR 1    ; Debug
NO_ERRCODE_ISR 2    ; Non-maskable interrupt
NO_ERRCODE_ISR 3    ; Breakpoint
NO_ERRCODE_ISR 4    ; Overflow
NO_ERRCODE_ISR 5    ; Bound range exceeded
NO_ERRCODE_ISR 6    ; Invalid opcode
NO_ERRCODE_ISR 7    ; Device not available
ERRCODE_ISR    8    ; Double fault
NO_ERRCODE_ISR 9    ; Coprocessor segment overrun
ERRCODE_ISR    10   ; Invalid TSS
ERRCODE_ISR    11   ; Segment not present
ERRCODE_ISR    12   ; Stack-segment fault
ERRCODE_ISR    13   ; General protection fault
ERRCODE_ISR    14   ; Page fault
NO_ERRCODE_ISR 15   ; Reserved
NO_ERRCODE_ISR 16   ; x87 floating-point exception
ERRCODE_ISR    17   ; Alignment check
NO_ERRCODE_ISR 18   ; Machine check
NO_ERRCODE_ISR 19   ; SIMD floating-point exception
NO_ERRCODE_ISR 20   ; Virtualization exception
ERRCODE_ISR    21   ; Control protection exception
NO_ERRCODE_ISR 22   ; Reserved
NO_ERRCODE_ISR 23   ; Reserved
NO_ERRCODE_ISR 24   ; Reserved
NO_ERRCODE_ISR 25   ; Reserved
NO_ERRCODE_ISR 26   ; Reserved
NO_ERRCODE_ISR 27   ; Reserved
NO_ERRCODE_ISR 28   ; Hypervisor injection exception
ERRCODE_ISR    29   ; VMM communication exception
ERRCODE_ISR    30   ; Security exception
NO_ERRCODE_ISR 31   ; Reserved

; IRQ handlers (remapped to 32-47)
NO_ERRCODE_ISR 0x20 ; Timer (IRQ0)
NO_ERRCODE_ISR 0x21 ; Keyboard (IRQ1)
NO_ERRCODE_ISR 0x22 ; Cascade (IRQ2)
NO_ERRCODE_ISR 0x23 ; COM2 (IRQ3)
NO_ERRCODE_ISR 0x24 ; COM1 (IRQ4)
NO_ERRCODE_ISR 0x25 ; LPT2 (IRQ5)
NO_ERRCODE_ISR 0x26 ; Floppy (IRQ6)
NO_ERRCODE_ISR 0x27 ; LPT1 (IRQ7)
NO_ERRCODE_ISR 0x28 ; RTC (IRQ8)
NO_ERRCODE_ISR 0x29 ; Free (IRQ9)
NO_ERRCODE_ISR 0x2A ; Free (IRQ10)
NO_ERRCODE_ISR 0x2B ; Free (IRQ11)
NO_ERRCODE_ISR 0x2C ; PS2 Mouse (IRQ12)
NO_ERRCODE_ISR 0x2D ; FPU (IRQ13)
NO_ERRCODE_ISR 0x2E ; Primary ATA (IRQ14)
NO_ERRCODE_ISR 0x2F ; Secondary ATA (IRQ15)