; Context switch implementation for SpectreOS
; This file handles saving and restoring CPU state during task switches

global perform_task_switch
extern current_task

; perform_task_switch(registers_t* old_regs, registers_t* new_regs)
; Saves current CPU state to old_regs and loads state from new_regs
perform_task_switch:
    push ebp
    mov ebp, esp
    
    ; Get pointers to register structures
    mov eax, [ebp + 8]  ; old_regs (current task)
    mov ebx, [ebp + 12] ; new_regs (next task)
    
    ; Save current state to old_regs if not NULL
    test eax, eax
    jz .load_new
    
    ; Save general purpose registers
    mov [eax + 0], edi
    mov [eax + 4], esi
    mov [eax + 8], ebp
    mov [eax + 12], esp
    mov [eax + 16], ebx
    mov [eax + 20], edx
    mov [eax + 24], ecx
    ; eax is already used, we'll save it last
    
    ; Save segment registers
    mov cx, ds
    mov [eax + 32], cx  ; ds
    
    ; Save return address as EIP
    mov ecx, [ebp + 4]
    mov [eax + 36], ecx ; eip
    
    ; Save CS
    mov cx, cs
    mov [eax + 40], cx
    
    ; Save EFLAGS
    pushfd
    pop ecx
    mov [eax + 44], ecx
    
    ; Save ESP and SS
    mov ecx, esp
    mov [eax + 48], ecx
    mov cx, ss
    mov [eax + 52], cx
    
    ; Now save eax
    push eax
    mov eax, [ebp - 4]  ; Original eax from stack
    mov ecx, [esp]
    mov [ecx + 28], eax
    pop eax

.load_new:
    ; Load new state from new_regs
    test ebx, ebx
    jz .done
    
    ; Load general purpose registers
    mov edi, [ebx + 0]
    mov esi, [ebx + 4]
    mov ebp, [ebx + 8]
    ; Don't load ESP yet
    ; Don't load EBX yet (we're using it)
    mov edx, [ebx + 20]
    mov ecx, [ebx + 24]
    mov eax, [ebx + 28]
    
    ; Load segment registers
    mov dx, [ebx + 32]
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx
    
    ; Prepare stack for iret
    mov esp, [ebx + 48]  ; Load new ESP
    
    ; Push values for iret: SS, ESP, EFLAGS, CS, EIP
    push dword [ebx + 52] ; SS
    push dword [ebx + 48] ; ESP
    push dword [ebx + 44] ; EFLAGS
    push dword [ebx + 40] ; CS
    push dword [ebx + 36] ; EIP
    
    ; Load remaining registers
    mov ebx, [ebx + 16]
    
    ; Return to new task
    iretd

.done:
    pop ebp
    ret