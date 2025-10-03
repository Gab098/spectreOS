; This function loads our IDT.
global idt_flush

idt_flush:
    mov eax, [esp+4]  ; Get the pointer to the IDT pointer structure
    lidt [eax]        ; Load the IDT pointer
    ret
