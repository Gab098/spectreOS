; Common stub for all ISRs
; This stub is jumped to by all ISRs. It saves the current CPU state,
; sets up the kernel data segments, calls the C interrupt handler,
; restores the CPU state, and returns from the interrupt.
global isr_common_stub ; Make this symbol globally visible
isr_common_stub:
    push ds                 ; Push data segment selector
    pusha                   ; Push all general purpose registers
    mov ax, 0x10            ; Load the kernel data segment selector (0x10 is data segment)
    mov ds, ax              ; Set data segment
    mov es, ax              ; Set extra segment
    mov fs, ax              ; Set F segment
    mov gs, ax              ; Set G segment
    mov ss, ax              ; Set stack segment

    mov eax, esp            ; Pass the stack pointer (which now points to the regs struct)
    push eax                ; Push the pointer to the regs struct as an argument
    call isr_handler        ; Call the C interrupt handler
    pop eax                 ; Pop the pointer to the regs struct

    popa                    ; Pop all general purpose registers
    pop ds                  ; Pop data segment selector
    add esp, 8              ; Clean up the interrupt number and error code
    iret                    ; Return from interrupt

; Declare the C interrupt handler
extern isr_handler

; ISRs for exceptions (0-31)
; These do not push an error code
%macro NO_ERRCODE_ISR 1
    global isr%1
    isr%1:
        cli                 ; Disable interrupts
        push byte 0         ; Push a dummy error code
        push byte %1        ; Push the interrupt number
        jmp isr_common_stub
%endmacro

; These ISRs push an error code
%macro ERRCODE_ISR 1
    global isr%1
    isr%1:
        cli                 ; Disable interrupts
        ; Error code is already on the stack
        push byte %1        ; Push the interrupt number
        jmp isr_common_stub
%endmacro

NO_ERRCODE_ISR 0
NO_ERRCODE_ISR 1
NO_ERRCODE_ISR 2
NO_ERRCODE_ISR 3
NO_ERRCODE_ISR 4
NO_ERRCODE_ISR 5
NO_ERRCODE_ISR 6
NO_ERRCODE_ISR 7

ERRCODE_ISR 8

NO_ERRCODE_ISR 9
ERRCODE_ISR 10
ERRCODE_ISR 11
ERRCODE_ISR 12
ERRCODE_ISR 13
ERRCODE_ISR 14

NO_ERRCODE_ISR 15
NO_ERRCODE_ISR 16

ERRCODE_ISR 17

NO_ERRCODE_ISR 18
NO_ERRCODE_ISR 19
NO_ERRCODE_ISR 20

ERRCODE_ISR 21

NO_ERRCODE_ISR 22
NO_ERRCODE_ISR 23
NO_ERRCODE_ISR 24
NO_ERRCODE_ISR 25
NO_ERRCODE_ISR 26
NO_ERRCODE_ISR 27
NO_ERRCODE_ISR 28
NO_ERRCODE_ISR 29

ERRCODE_ISR 30

NO_ERRCODE_ISR 31

NO_ERRCODE_ISR 0x20 ; Timer IRQ (remapped to 0x20)
NO_ERRCODE_ISR 0x21 ; Keyboard IRQ (remapped to 0x21)
