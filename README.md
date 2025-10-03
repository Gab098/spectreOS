# Progetto SpectreOS: Un Sistema Operativo Modulare per il Pentesting

Questo documento delinea l'architettura e la roadmap per lo sviluppo di SpectreOS, un sistema operativo a 64-bit progettato da zero con un focus sulla modularità, personalizzazione e massima efficacia per il penetration testing e l'hacking etico.

## Filosofia di Progettazione

- **Architettura a Microkernel Ibrido:** Il kernel si occupa solo delle operazioni più critiche (memoria, scheduling, IPC). Tutto il resto, inclusi driver, stack di rete e file system, viene eseguito come server isolati nello spazio utente.
- **Modularità Estrema:** Ogni componente del sistema può essere sostituito, riavviato o modificato al volo senza compromettere la stabilità del sistema.
- **Controllo a Basso Livello:** Fornire agli strumenti di pentesting un accesso diretto e non filtrato all'hardware, in particolare allo stack di rete.
- **Interfacce Flessibili:** Supportare sia una shell tradizionale per lo scripting che un'interfaccia in linguaggio naturale per comandi più intuitivi.

## Roadmap di Sviluppo

Il progetto è suddiviso in fasi incrementali.

### Fase 1: Il Nucleo (Microkernel)
L'obiettivo di questa fase è creare un kernel minimo in grado di essere avviato e di eseguire codice C.
- **[x] Bootloader:** Utilizzeremo GRUB con lo standard Multiboot2 per avviare il kernel.
- **[x] Kernel Entry Point:** Un piccolo bootstrap in Assembly per impostare lo stack e passare il controllo a una funzione C (`kmain`).
- **[x] Inizializzazione a 64-bit:** Configurazione delle strutture essenziali per la Long Mode (PML4, GDT, IDT).
- **[x] Gestione della Memoria:** Implementazione di un gestore di memoria fisica (per tenere traccia della RAM disponibile) e di un sistema di paginazione.
- [ ] Scheduler Preemptive: Un semplice scheduler per consentire l'esecuzione di più processi.
- [ ] Sistema IPC (Inter-Process Communication): Il cuore del sistema. Un meccanismo di messaggistica efficiente per la comunicazione tra i server.

### Fase 2: I Server di Base
Una volta che il kernel è funzionante, inizieremo a costruire i servizi fondamentali come processi separati.
- **[x] Server del Terminale:** Per l'output di testo sullo schermo.
- **[x] Server del Driver della Tastiera:** Per l'input utente.
- [ ] Server del Bus PCI: Per enumerare l'hardware collegato al sistema.

### Fase 3: Lo Stack di Rete Modulare
Questa è la fase più critica per il caso d'uso di pentesting.
- [ ] Server del Driver di Rete: Un driver per una scheda di rete comune (es. Intel E1000 in QEMU).
- [ ] Server dei Protocolli: Implementazione dei livelli dello stack di rete (Ethernet, ARP, IP, TCP, UDP) come server separati.
- [ ] API per Raw Sockets: Creazione di un'interfaccia IPC che permetta a qualsiasi applicazione di creare e iniettare pacchetti a qualsiasi livello e di sniffare tutto il traffico.

### Fase 4: Storage e Userspace di Base
- [ ] Server del File System: Un driver per un file system semplice (es. FAT32).
- **[ ] Shell Semplice:** Una shell di base per lanciare comandi e interagire con il sistema.

### Fase 5: Shell Avanzate
- **[ ] Porting di una Shell Standard:** Porting di una shell come `bash` o `dash`.
- **[ ] Sviluppo della NLP Shell:** Creazione di una shell che interpreta comandi in linguaggio naturale.

### Fase 6: Il Sistema Grafico (GUI)
- **[ ] Server Grafico:** Un driver a basso livello per la scheda video.
- **[ ] Window Server / Compositor:** Il server che gestisce le finestre, gli eventi di input e il rendering.
- **[ ] Desktop Environment:** Le applicazioni di base che compongono l'esperienza utente grafica.

## Stack Tecnologico
- **Linguaggi:** C (Kernel, Server), Assembly (Bootstrap)
- **Architettura:** x86-64
- **Bootloader:** GRUB (con Multiboot2)
- **Toolchain di Compilazione:** GCC, NASM, LD
- **Ambiente di Test:** QEMU
