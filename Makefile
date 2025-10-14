# Define the compilers and assemblers
AS = nasm
CC = gcc
LD = ld

# Define compiler and linker flags
ASFLAGS = -felf32 -g

# Add -O2 for optimization, -Wall for warnings
CFLAGS = -ffreestanding -nostdlib -g -m32 -O2 -Wall -Wextra -fno-builtin -fno-stack-protector

LDFLAGS = -T src/linker.ld -m elf_i386

# Define source files
AS_SOURCES = src/multiboot_header.asm src/bootstrap.asm src/gdt_asm.asm \
             src/idt_handlers.asm src/idt_flush.asm src/context_switch.asm \
             src/syscall.asm

C_SOURCES = src/kernel.c src/gdt.c src/idt.c src/pic.c src/keyboard.c \
            src/mm.c src/task.c src/heap.c src/process.c src/serial.c \
            src/vfs.c src/string.c src/syscall.c src/usermode.c src/scheduler.c

# Define object files
AS_OBJECTS = build/multiboot_header.o build/bootstrap.o build/gdt_asm.o \
             build/idt_handlers.o build/idt_flush.o build/context_switch.o \
             build/syscall_isr.o

C_OBJECTS = $(patsubst src/%.c, build/%.o, $(C_SOURCES))
OBJECTS = $(AS_OBJECTS) $(C_OBJECTS)

# The final kernel binary
KERNEL = build/kernel.bin

# Default target
all: $(KERNEL)

# Rule to build the kernel
$(KERNEL): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $(KERNEL) $(OBJECTS)

# Rule to compile assembly files
build/multiboot_header.o: src/multiboot_header.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/bootstrap.o: src/bootstrap.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/gdt_asm.o: src/gdt_asm.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/idt_handlers.o: src/idt_handlers.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/idt_flush.o: src/idt_flush.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/context_switch.o: src/context_switch.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

build/syscall_isr.o: src/syscall.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@


# Rule to compile C files
$(C_OBJECTS): build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf build

# --- ISO Creation ---
ISO = build/kernel.iso

iso: $(KERNEL)
	mkdir -p build/iso/boot/grub
	cp $(KERNEL) build/iso/boot/kernel.bin
	cp src/grub.cfg build/iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) build/iso

# Rule to run the kernel in QEMU
run: iso
	qemu-system-i386 -cdrom $(ISO) -serial stdio

# Debug target with GDB
debug: iso
	qemu-system-i386 -cdrom $(ISO) -s -S -serial stdio

.PHONY: all clean run iso debug