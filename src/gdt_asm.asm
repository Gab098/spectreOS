; This function loads our GDT.
global gdt_flush
extern gdt_ptr

gdt_flush:
    mov eax, [esp+4]  ; Get the pointer to the GDT pointer structure
    lgdt [eax]        ; Load the GDT pointer

    ; Reload the segment registers to use the new GDT.
    ; 0x10 is the offset of our data segment descriptor in the GDT.
    ; 0x08 is the offset of our code segment descriptor.
    mov ax, 0x10      ; Load the data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; Far jump to the code segment to reload CS.
.flush:
    ret
