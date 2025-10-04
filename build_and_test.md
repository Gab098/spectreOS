# SpectreOS - Istruzioni di Compilazione e Test

## Prerequisiti

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential nasm qemu-system-x86 grub-pc-bin xorriso
```

### Fedora/RHEL
```bash
sudo dnf install gcc nasm qemu-system-x86 grub2-tools xorriso
```

### Arch Linux
```bash
sudo pacman -S base-devel nasm qemu grub xorriso
```

## Compilazione

### Build completo
```bash
make clean
make all
```

### Creare l'immagine ISO
```bash
make iso
```

### Build e esecuzione in un comando
```bash
make run
```

## Test e Debug

### Esecuzione normale
```bash
make run
```
Questo compila, crea l'ISO ed esegue QEMU con output seriale su stdio.

### Debug con GDB
Terminal 1:
```bash
make debug
```

Terminal 2:
```bash
gdb build/kernel.bin
(gdb) target remote localhost:1234
(gdb) break kmain
(gdb) continue
```

### Comandi GDB utili
```gdb
# Breakpoint sulla funzione kmain
break kmain

# Breakpoint su init_tasking
break init_tasking

# Breakpoint sul timer handler
break schedule_and_switch

# Stampa lo stato del task corrente
print *current_task

# Stampa tutti i task nella ready queue
print *ready_queue

# Step attraverso il codice
step

# Continue esecuzione
continue

# Backtrace dello stack
backtrace
```

## Verificare il Funzionamento

### Output Atteso

All'avvio dovresti vedere:
```
==================================
  Benvenuto in SpectreOS v0.2
  Multitasking Operating System
==================================

Inizializzazione Debug Seriale...
Debug seriale attivo su COM1.
Inizializzazione PMM...
PMM inizializzato.
Inizializzazione VMM...
VMM inizializzato.
Inizializzazione Heap...
Heap inizializzato.
Inizializzazione GDT...
GDT inizializzata.
Inizializzazione IDT...
IDT inizializzata.
Inizializzazione PIC...
PIC inizializzato.
Inizializzazione Timer...
Timer inizializzato a 100 Hz.
Inizializzazione VFS...
VFS (RAMFS) inizializzato.

=== Testing VFS ===
Created file: test.txt
Wrote 25 bytes
Read 25 bytes: Hello from SpectreOS VFS!
Created directory: testdir
Created file: testdir/subfile.txt

Contents of /:
  test.txt
  testdir [DIR]
=== VFS Test Complete ===

Inizializzazione Multitasking...
Sistema di tasking inizializzato.
Interrupt abilitati.

Creazione task di test...
Task creati con successo!
Task 1 PID: 2
Task 2 PID: 3
Task 3 PID: 4

==================================
 Kernel avviato con successo!
 Multitasking attivo.
 Premere tasti per testare input.
==================================

111222333111222333...
```

### Multitasking Verification

Dovresti vedere i caratteri '1', '2' e '3' apparire sullo schermo in colori diversi:
- '1' in verde (task 1)
- '2' in ciano (task 2)
- '3' in giallo (task 3)

Questo dimostra che il context switching funziona e che i task vengono schedulati correttamente.

### Keyboard Test

Premi alcuni tasti sulla tastiera e dovresti vedere i caratteri apparire sullo schermo.

### Serial Output

Sul terminale (se usi `make run`) vedrai anche l'output della porta seriale con messaggi di debug.

## Troubleshooting

### Problema: Kernel non si avvia
- Verifica che GRUB sia installato correttamente
- Controlla che il file `build/kernel.bin` sia stato creato
- Verifica il linker script `src/linker.ld`

### Problema: Page Fault all'avvio
- Probabile problema con la memoria virtuale
- Verifica che PMM e VMM siano inizializzati nell'ordine corretto
- Debug con: `break vmm_init` in GDB

### Problema: Task non switchano
- Verifica che il timer sia configurato correttamente
- Verifica che gli interrupt siano abilitati (`sti`)
- Debug con: `break schedule_and_switch`

### Problema: Triple Fault (riavvio continuo)
- Controlla che GDT e IDT siano configurate correttamente
- Verifica che non ci siano page fault non gestiti
- Usa QEMU con `-d int,cpu_reset` per vedere cosa causa il triple fault:
  ```bash
  qemu-system-i386 -cdrom build/kernel.iso -d int,cpu_reset -no-reboot
  ```

### Problema: Heap corrotto
- Usa `heap_dump()` per vedere lo stato dell'heap
- Verifica che non ci siano double free o buffer overflow
- Controlla i magic numbers nei blocchi

## Performance Testing

### Misurare il Context Switch
Puoi misurare quanti context switch avvengono al secondo modificando `task.c` per contare gli switch e stampare stats ogni secondo.

### Memory Usage
Aggiungi queste funzioni per monitorare l'uso della memoria:
```c
void pmm_stats() {
    // Conta frame liberi
    uint32_t free_frames = 0;
    for (uint32_t i = 0; i < pmm_total_frames; i++) {
        if (!pmm_test_bit(i)) free_frames++;
    }
    print("Free frames: ");
    print_dec(free_frames);
    print("/");
    print_dec(pmm_total_frames);
    print("\n");
}
```

## Prossimi Passi

Dopo aver verificato che il multitasking funziona:

1. **Implementare syscall** per comunicazione user/kernel
2. **User mode** per eseguire task in ring 3
3. **ELF loader** per caricare programmi da disco
4. **Scheduler più avanzato** (priority, real-time)
5. **IPC (Inter-Process Communication)** per comunicazione tra task
6. **Network stack** seguendo la roadmap di SpectreOS

## Risorse Utili

- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel 64 and IA-32 Architectures Software Developer's Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
- [QEMU Documentation](https://www.qemu.org/docs/master/)
- [GDB Quick Reference](https://sourceware.org/gdb/current/onlinedocs/gdb/)