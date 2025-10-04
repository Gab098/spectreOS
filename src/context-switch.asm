; Context switch implementation
; This function performs the actual task switch

global switch_to_task
extern current_task

; void switch_to_task(registers_t* old_regs, registers_t* new_regs)
switch_to_task:
    push ebp
    mov ebp, esp
    
    ; Save current task state (if old_regs is not NULL)
    mov eax, [ebp + 8]  ; old_regs pointer
    test eax, eax
    jz .load_new_state
    
    ; Save current registers to old_regs
    mov [eax + 0], edi
    mov [eax + 4], esi
    mov [eax + 8], ebp
    mov [eax + 12], esp
    mov [eax + 16], ebx
    mov [eax + 20], edx
    mov [eax + 24], ecx
    mov [eax + 28], ecx  ; eax will be saved later
    
.load_new_state:
    ; Load new task state
    mov eax, [ebp + 12]  ; new_regs pointer
    
    ; Restore registers from new_regs
    mov edi, [eax + 0]
    mov esi, [eax + 4]
    mov ebp, [eax + 8]
    mov esp, [eax + 12]
    mov ebx, [eax + 16]
    mov edx, [eax + 20]
    mov ecx, [eax + 24]
    ; eax is restored last
    
    ; Restore segment registers
    mov ax, [eax + 32 + 2]  ; ds
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; The actual context switch will happen via iret in the interrupt handler
    pop ebp
    ret