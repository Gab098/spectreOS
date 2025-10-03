# Define the compilers and assemblers
AS = nasm
CC = gcc
LD = ld

# Define compiler and linker flags
ASFLAGS = -felf32 -g

# -ffreestanding: we are not linking against a standard library
# -nostdlib: don't link standard library
# -g: add debug info
# -m32: compile for 32-bit
# -fno-pie: disable position independent executable
# -fno-stack-protector: disable stack protection (not available in freestanding)
CFLAGS = -ffreestanding -nostdlib -g -m32 -fno-pie -fno-stack-protector -Wall -Wextra

# -T: use our linker script
# -m elf_i386: output format
LDFLAGS = -T src/linker.ld -m elf_i386

# Define source files
AS_SOURCES = src/multiboot_header.asm \
             src/bootstrap.asm \
             src/gdt_asm.asm \
             src/idt_handlers.asm \
             src/idt_flush.asm \
             src/context_switch.asm

C_SOURCES = src/kernel.c \
            src/gdt.c \
            src/idt.c \
            src/pic.c \
            src/keyboard.c \
            src/mm.c \
            src/task.c \
            src/heap.c \
            src/process.c \
            src/serial.c \
            src/vfs.c \
            src/string.c

# Define object files
AS_OBJECTS = $(patsubst src/%.asm, build/%.o, $(AS_SOURCES))
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
build/%.o: src/%.asm
	mkdir -p build
	$(AS) $(ASFLAGS) $< -o $@

# Rule to compile C files
build/%.o: src/%.c
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

# Rule to run with debugging
debug: iso
	qemu-system-i386 -cdrom $(ISO) -serial stdio -s -S

.PHONY: all clean run iso debug