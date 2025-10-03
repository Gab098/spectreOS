# Mappa dei File di SpectreOS

Questo documento fornisce una panoramica dei file principali nel progetto SpectreOS e delle loro funzionalità.

## File di Root

- `Makefile`: Contiene le istruzioni per la compilazione del kernel, la creazione dell'immagine ISO avviabile e l'esecuzione in QEMU. Definisce i compilatori, i flag, i file sorgente e oggetto, e le regole per la pulizia del progetto.
- `README.md`: Descrive la filosofia di progettazione, la roadmap di sviluppo e lo stack tecnologico di SpectreOS, un sistema operativo modulare per il pentesting.

## Directory `src/`

- `src/bootstrap.asm`: Il punto di ingresso iniziale del kernel in Assembly a 32-bit. Imposta lo stack, disabilita gli interrupt e chiama la funzione `kmain` del kernel C.
- `src/gdt_asm.asm`: Contiene la funzione `gdt_flush` in Assembly, responsabile del caricamento della Global Descriptor Table (GDT) e del ricaricamento dei registri di segmento.
- `src/gdt.c`: Implementa la logica per la creazione e l'inizializzazione della Global Descriptor Table (GDT), che definisce i segmenti di memoria per il kernel.
- `src/gdt.h`: Contiene le definizioni delle strutture `gdt_entry_t` e `gdt_ptr_t` e le dichiarazioni delle funzioni relative alla GDT.
- `src/grub.cfg`: Il file di configurazione per il bootloader GRUB, che indica a GRUB come avviare il kernel `kernel.bin`.
- `src/idt_flush.asm`: Contiene la funzione `idt_flush` in Assembly, responsabile del caricamento della Interrupt Descriptor Table (IDT).
- `src/idt_handlers.asm`: Contiene gli stub in Assembly per tutti gli Interrupt Service Routines (ISR) e Interrupt Request (IRQ) handlers. Questi stub salvano lo stato della CPU, chiamano il gestore C e ripristinano lo stato. Include anche gestori dedicati per il timer (IRQ0) e la tastiera (IRQ1).
- `src/idt.c`: Implementa la logica per la creazione e l'inizializzazione della Interrupt Descriptor Table (IDT), che mappa gli interrupt a specifici gestori. Include il gestore generico `isr_handler` per eccezioni e IRQ.
- `src/idt.h`: Contiene le definizioni delle strutture `idt_entry_t`, `idt_ptr_t` e `regs` (per lo stato della CPU durante un interrupt), e le dichiarazioni delle funzioni e degli ISR/IRQ handlers.
- `src/io.h`: Fornisce funzioni inline per l'input/output (I/O) a basso livello, come `inb` (read byte from port), `outb` (write byte to port) e `io_wait`.
- `src/kernel.c`: Il file sorgente principale del kernel C. Contiene la funzione `kmain` (il punto di ingresso del kernel C), funzioni per la stampa a schermo (`k_print_char`, `print`) e l'inizializzazione di GDT, IDT e PIC.
- `src/keyboard.c`: Implementa il driver della tastiera. Contiene una tabella di conversione da scancode a ASCII e la funzione `keyboard_handler` che elabora gli input da tastiera.
- `src/keyboard.h`: Contiene le definizioni delle porte del controller della tastiera e le dichiarazioni delle funzioni del driver della tastiera.
- `src/linker.ld`: Lo script del linker che definisce come le diverse sezioni del kernel (come `.text`, `.rodata`, `.data`, `.bss`) devono essere disposte nella memoria.
- `src/multiboot_header.asm`: Contiene l'intestazione Multiboot2 in Assembly, necessaria per comunicare con il bootloader GRUB e indicare che il kernel è conforme allo standard Multiboot2.
- `src/pic.c`: Implementa la logica per l'inizializzazione e la rimappatura del Programmable Interrupt Controller (PIC), che gestisce gli interrupt hardware. Include anche la funzione `pic_send_eoi` per inviare il comando End-Of-Interrupt.
- `src/pic.h`: Contiene le definizioni delle porte del PIC e le dichiarazioni delle funzioni per la sua gestione.

## Directory `build/`

Questa directory contiene i file generati durante il processo di compilazione.

- `build/*.o`: File oggetto intermedi generati dalla compilazione dei file `.asm` e `.c`.
- `build/kernel.bin`: Il binario eseguibile del kernel compilato.
- `build/kernel.iso`: L'immagine ISO avviabile contenente il kernel e la configurazione di GRUB, utilizzabile per l'avvio in una macchina virtuale come QEMU.
- `build/iso/`: Directory temporanea utilizzata per costruire la struttura dell'immagine ISO.
