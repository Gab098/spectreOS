; context_switch.asm
; Low-level context switching for task management

global switch_to_task
extern current_task

; void switch_to_task(task_t* next_task)
; This function performs a complete context switch to the next task
switch_to_task:
    cli                         ; Disable interrupts during switch
    
    mov eax, [esp + 4]          ; Get next_task pointer from argument
    
    ; Save current task context (if we have a current task)
    mov ebx, [current_task]
    test ebx, ebx
    jz .load_context            ; If no current task, just load new one
    
    ; Save current registers into current_task structure
    ; Offset 0: edi, 4: esi, 8: ebp, 12: esp, 16: ebx, 20: edx, 24: ecx, 28: eax
    mov [ebx + 0], edi
    mov [ebx + 4], esi
    mov [ebx + 8], ebp
    mov [ebx + 12], esp
    mov [ebx + 16], ebx
    mov [ebx + 20], edx
    mov [ebx + 24], ecx
    mov [ebx + 28], eax
    
    ; Save segment registers (offset 32+)
    mov cx, ds
    mov [ebx + 32], cx
    mov cx, es
    mov [ebx + 34], cx
    mov cx, fs
    mov [ebx + 36], cx
    mov cx, gs
    mov [ebx + 38], cx
    
.load_context:
    ; Update current_task pointer
    mov [current_task], eax
    
    ; Restore registers from new task structure
    mov edi, [eax + 0]
    mov esi, [eax + 4]
    mov ebp, [eax + 8]
    mov esp, [eax + 12]
    mov ebx, [eax + 16]
    mov edx, [eax + 20]
    mov ecx, [eax + 24]
    ; Don't restore eax yet, we need it
    
    ; Restore segment registers
    mov cx, [eax + 32]
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx
    
    ; Finally restore eax
    mov eax, [eax + 28]
    
    sti                         ; Re-enable interrupts
    ret

; Alternative: Simplified switch that just changes ESP
; Useful for cooperative multitasking
global switch_stack
switch_stack:
    mov eax, [esp + 4]          ; Get new stack pointer
    mov esp, eax                ; Switch stack
    ret