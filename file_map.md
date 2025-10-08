# Mappa dei File di SpectreOS

Questo documento fornisce una panoramica dettagliata dei file principali nel progetto SpectreOS, organizzati per funzionalità e directory, per facilitare la comprensione della struttura del sistema operativo.

## File di Root

- `Makefile`: Contiene le istruzioni per la compilazione del kernel, la creazione dell'immagine ISO avviabile e l'esecuzione in QEMU. Definisce i compilatori, i flag, i file sorgente e oggetto, e le regole per la pulizia del progetto.
- `README.md`: Descrive la filosofia di progettazione, la roadmap di sviluppo e lo stack tecnologico di SpectreOS, un sistema operativo modulare per il pentesting.

## Directory `src/` - Codice Sorgente del Kernel

Questa directory contiene tutti i file sorgente del kernel, suddivisi in categorie logiche.

### Inizialializzazione e Bootloader

- `src/bootstrap.asm`: Il punto di ingresso iniziale del kernel in Assembly a 32-bit. Imposta lo stack, disabilita gli interrupt e chiama la funzione `kmain` del kernel C.
- `src/grub.cfg`: Il file di configurazione per il bootloader GRUB, che indica a GRUB come avviare il kernel `kernel.bin`.
- `src/multiboot_header.asm`: Contiene l'intestazione Multiboot2 in Assembly, necessaria per comunicare con il bootloader GRUB e indicare che il kernel è conforme allo standard Multiboot2.
- `src/multiboot2.h`: Contiene le definizioni delle strutture e dei flag per l'intestazione Multiboot2, utilizzate per comunicare informazioni dal bootloader al kernel.
- `src/linker.ld`: Lo script del linker che definisce come le diverse sezioni del kernel (come `.text`, `.rodata`, `.data`, `.bss`) devono essere disposte nella memoria.
- `src/kernel.c`: Il file sorgente principale del kernel C. Contiene la funzione `kmain` (il punto di ingresso del kernel C), funzioni per la stampa a schermo (`k_print_char`, `print`) e l'inizializzazione di GDT, IDT e PIC.

### Gestione della Memoria

- `src/gdt_asm.asm`: Contiene la funzione `gdt_flush` in Assembly, responsabile del caricamento della Global Descriptor Table (GDT) e del ricaricamento dei registri di segmento.
- `src/gdt.c`: Implementa la logica per la creazione e l'inizializzazione della Global Descriptor Table (GDT), che definisce i segmenti di memoria per il kernel.
- `src/gdt.h`: Contiene le definizioni delle strutture `gdt_entry_t` e `gdt_ptr_t` e le dichiarazioni delle funzioni relative alla GDT.
- `src/heap.c`: Implementa un gestore di memoria heap per l'allocazione dinamica della memoria all'interno del kernel.
- `src/heap.h`: Contiene le dichiarazioni delle funzioni e delle strutture dati per la gestione dell'heap.
- `src/mm.c`: Implementa la gestione della memoria fisica e virtuale, inclusa la paginazione e l'allocazione di pagine.
- `src/mm.h`: Contiene le definizioni delle strutture e delle funzioni per la gestione della memoria.

### Gestione degli Interrupt

- `src/idt_flush.asm`: Contiene la funzione `idt_flush` in Assembly, responsabile del caricamento della Interrupt Descriptor Table (IDT).
- `src/idt_handlers.asm`: Contiene gli stub in Assembly per tutti gli Interrupt Service Routines (ISR) e Interrupt Request (IRQ) handlers. Questi stub salvano lo stato della CPU, chiamano il gestore C e ripristinano lo stato. Include anche gestori dedicati per il timer (IRQ0) e la tastiera (IRQ1).
- `src/idt.c`: Implementa la logica per la creazione e l'inizializzazione della Interrupt Descriptor Table (IDT), che mappa gli interrupt a specifici gestori. Include il gestore generico `isr_handler` per eccezioni e IRQ.
- `src/idt.h`: Contiene le definizioni delle strutture `idt_entry_t`, `idt_ptr_t` e `regs` (per lo stato della CPU durante un interrupt), e le dichiarazioni delle funzioni e degli ISR/IRQ handlers.
- `src/pic.c`: Implementa la logica per l'inizializzazione e la rimappatura del Programmable Interrupt Controller (PIC), che gestisce gli interrupt hardware. Include anche la funzione `pic_send_eoi` per inviare il comando End-Of-Interrupt.
- `src/pic.h`: Contiene le definizioni delle porte del PIC e le dichiarazioni delle funzioni per la sua gestione.

### Gestione dei Processi e Task

- `src/context-switch.asm`: Contiene le routine in Assembly per il salvataggio e il ripristino del contesto della CPU, essenziali per il multitasking e il cambio di processo.
- `src/process.c`: Implementa la gestione dei processi, inclusa la creazione, la terminazione e la schedulazione dei processi.
- `src/process.h`: Contiene le definizioni delle strutture `process_t` e le dichiarazioni delle funzioni per la gestione dei processi.
- `src/scheduler.h`: Contiene le dichiarazioni delle funzioni e delle strutture dati per lo scheduler del kernel.
- `src/task.c`: Implementa la gestione dei task (thread), inclusa la creazione, la terminazione e il cambio di contesto dei task.
- `src/task.h`: Contiene le definizioni delle strutture `task_t` e le dichiarazioni delle funzioni per la gestione dei task.
- `src/usermode.h`: Contiene le definizioni e le dichiarazioni relative al passaggio alla modalità utente.

### Driver e I/O

- `src/io.h`: Fornisce funzioni inline per l'input/output (I/O) a basso livello, come `inb` (read byte from port), `outb` (write byte to port) e `io_wait`.
- `src/keyboard.c`: Implementa il driver della tastiera. Contiene una tabella di conversione da scancode a ASCII e la funzione `keyboard_handler` che elabora gli input da tastiera.
- `src/keyboard.h`: Contiene le definizioni delle porte del controller della tastiera e le dichiarazioni delle funzioni del driver della tastiera.
- `src/serial.c`: Implementa il driver per la comunicazione seriale (UART), utilizzato per il debug e l'output della console.
- `src/serial.h`: Contiene le definizioni delle porte seriali e le dichiarazioni delle funzioni del driver seriale.
- `src/vfs.c`: Implementa il Virtual File System (VFS), fornendo un'interfaccia unificata per l'accesso a diversi filesystem.
- `src/vfs.h`: Contiene le definizioni delle strutture e delle funzioni per il Virtual File System.

### Utilità

- `src/string.c`: Implementa funzioni di manipolazione delle stringhe (es. `strlen`, `strcmp`, `strcpy`) per l'uso nel kernel.
- `src/string.h`: Contiene le dichiarazioni delle funzioni di manipolazione delle stringhe.
- `src/syscall.h`: Contiene le definizioni delle chiamate di sistema (syscalls) e le dichiarazioni delle funzioni correlate.

## Directory `build/` - Artefatti di Compilazione

Questa directory contiene i file generati durante il processo di compilazione.

- `build/*.o`: File oggetto intermedi generati dalla compilazione dei file `.asm` e `.c`.
- `build/kernel.bin`: Il binario eseguibile del kernel compilato.
- `build/kernel.iso`: L'immagine ISO avviabile contenente il kernel e la configurazione di GRUB, utilizzabile per l'avvio in una macchina virtuale come QEMU.
- `build/iso/`: Directory temporanea utilizzata per costruire la struttura dell'immagine ISO.
