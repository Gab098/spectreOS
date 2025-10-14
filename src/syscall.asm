; syscall.asm - System call interrupt handler
global isr128
extern syscall_dispatcher

; System call handler (interrupt 0x80)
isr128:
    cli                     ; Disable interrupts
    
    ; Save all registers
    push 0                  ; Push dummy error code 
    push 0x80               ; Push interrupt number
    
    pusha                   ; Push all general registers
    
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call syscall dispatcher
    ; Arguments are already in registers (Linux ABI):
    ; eax = syscall number
    ; ebx = arg1, ecx = arg2, edx = arg3, esi = arg4, edi = arg5
    push edi                ; arg5
    push esi                ; arg4
    push edx                ; arg3
    push ecx                ; arg2
    push ebx                ; arg1
    push eax                ; syscall number
    
    call syscall_dispatcher
    
    ; Clean up stack (6 arguments * 4 bytes)
    add esp, 24
    
    ; Store return value in the saved eax location
    ; The saved registers are at [esp]
    mov [esp + 32], eax     ; Overwrite saved eax with return value
    
    ; Restore segments
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restore registers (eax will have the return value)
    popa
    
    ; Remove error code and interrupt number
    add esp, 8
    
    ; Return from interrupt
    sti
    iret