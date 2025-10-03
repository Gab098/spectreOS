; Common stub for all ISRs
; This stub saves the CPU state, calls the C handler, and restores state
global isr_common_stub
extern isr_handler

isr_common_stub:
    ; Save all general purpose registers
    pusha                   ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
    
    ; Save data segment
    mov ax, ds
    push eax
    
    ; Load kernel data segment
    mov ax, 0x10            ; Kernel data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C handler with pointer to register struct
    push esp                ; ESP now points to struct regs
    call isr_handler
    add esp, 4              ; Clean up pushed ESP
    
    ; Restore data segment
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore general purpose registers
    popa
    
    ; Clean up error code and interrupt number
    add esp, 8
    
    ; Return from interrupt
    iret

; Macro for ISRs that don't push an error code
%macro NO_ERRCODE_ISR 1
    global isr%1
    isr%1:
        cli
        push byte 0         ; Dummy error code
        push byte %1        ; Interrupt number
        jmp isr_common_stub
%endmacro

; Macro for ISRs that push an error code
%macro ERRCODE_ISR 1
    global isr%1
    isr%1:
        cli
        ; Error code already pushed by CPU
        push byte %1        ; Interrupt number
        jmp isr_common_stub
%endmacro

; CPU Exception handlers (0-31)
NO_ERRCODE_ISR 0    ; Divide by zero
NO_ERRCODE_ISR 1    ; Debug
NO_ERRCODE_ISR 2    ; Non-maskable interrupt
NO_ERRCODE_ISR 3    ; Breakpoint
NO_ERRCODE_ISR 4    ; Overflow
NO_ERRCODE_ISR 5    ; Bound range exceeded
NO_ERRCODE_ISR 6    ; Invalid opcode
NO_ERRCODE_ISR 7    ; Device not available

ERRCODE_ISR 8       ; Double fault

NO_ERRCODE_ISR 9    ; Coprocessor segment overrun

ERRCODE_ISR 10      ; Invalid TSS
ERRCODE_ISR 11      ; Segment not present
ERRCODE_ISR 12      ; Stack-segment fault
ERRCODE_ISR 13      ; General protection fault
ERRCODE_ISR 14      ; Page fault

NO_ERRCODE_ISR 15   ; Reserved
NO_ERRCODE_ISR 16   ; x87 floating-point exception

ERRCODE_ISR 17      ; Alignment check

NO_ERRCODE_ISR 18   ; Machine check
NO_ERRCODE_ISR 19   ; SIMD floating-point exception
NO_ERRCODE_ISR 20   ; Virtualization exception

ERRCODE_ISR 21      ; Control protection exception

NO_ERRCODE_ISR 22   ; Reserved
NO_ERRCODE_ISR 23   ; Reserved
NO_ERRCODE_ISR 24   ; Reserved
NO_ERRCODE_ISR 25   ; Reserved
NO_ERRCODE_ISR 26   ; Reserved
NO_ERRCODE_ISR 27   ; Reserved
NO_ERRCODE_ISR 28   ; Hypervisor injection exception
NO_ERRCODE_ISR 29   ; VMM communication exception

ERRCODE_ISR 30      ; Security exception

NO_ERRCODE_ISR 31   ; Reserved

; Hardware interrupt handlers (IRQs remapped to 32-47)
NO_ERRCODE_ISR 32   ; Timer (IRQ0)
NO_ERRCODE_ISR 33   ; Keyboard (IRQ1)
NO_ERRCODE_ISR 34   ; IRQ2
NO_ERRCODE_ISR 35   ; IRQ3
NO_ERRCODE_ISR 36   ; IRQ4
NO_ERRCODE_ISR 37   ; IRQ5
NO_ERRCODE_ISR 38   ; IRQ6
NO_ERRCODE_ISR 39   ; IRQ7
NO_ERRCODE_ISR 40   ; IRQ8
NO_ERRCODE_ISR 41   ; IRQ9
NO_ERRCODE_ISR 42   ; IRQ10
NO_ERRCODE_ISR 43   ; IRQ11
NO_ERRCODE_ISR 44   ; IRQ12
NO_ERRCODE_ISR 45   ; IRQ13
NO_ERRCODE_ISR 46   ; IRQ14
NO_ERRCODE_ISR 47   ; IRQ15