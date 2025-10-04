#include "vfs.h"
#include "heap.h"
#include "string.h"
#include <stddef.h>

vfs_node_t* fs_root = NULL;

// Implementazioni delle funzioni VFS wrapper

uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node || !node->read) {
        return 0;
    }
    return node->read(node, offset, size, buffer);
}

uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (!node || !node->write) {
        return 0;
    }
    return node->write(node, offset, size, buffer);
}

void vfs_open(vfs_node_t* node, uint8_t read, uint8_t write) {
    if (!node || !node->open) {
        return;
    }
    node->open(node);
}

void vfs_close(vfs_node_t* node) {
    if (!node || !node->close) {
        return;
    }
    node->close(node);
}

vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (!node || !node->readdir) {
        return NULL;
    }
    return node->readdir(node, index);
}

vfs_node_t* vfs_finddir(vfs_node_t* node, char* name) {
    if (!node || !node->finddir) {
        return NULL;
    }
    return node->finddir(node, name);
}

// Funzione per creare un nuovo nodo VFS
vfs_node_t* vfs_create_node(const char* name, uint32_t flags) {
    vfs_node_t* node = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!node) {
        return NULL;
    }
    
    memset(node, 0, sizeof(vfs_node_t));
    strncpy(node->name, name, 127);
    node->name[127] = '\0';
    node->flags = flags;
    
    return node;
}

// Funzione per liberare un nodo VFS
void vfs_free_node(vfs_node_t* node) {
    if (node) {
        kfree(node);
    }
}

// === RAMFS: Un filesystem in memoria semplice per iniziare ===

#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02

typedef struct ramfs_entry {
    vfs_node_t* node;
    uint8_t* data;
    uint32_t size;
    struct ramfs_entry* next;
    struct ramfs_entry* children; // Per le directory
} ramfs_entry_t;

static ramfs_entry_t* ramfs_root_entry = NULL;

// Funzioni RAMFS
uint32_t ramfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    ramfs_entry_t* entry = (ramfs_entry_t*)node->ptr;
    if (!entry || !entry->data) {
        return 0;
    }
    
    if (offset >= entry->size) {
        return 0;
    }
    
    if (offset + size > entry->size) {
        size = entry->size - offset;
    }
    
    memcpy(buffer, entry->data + offset, size);
    return size;
}

uint32_t ramfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    ramfs_entry_t* entry = (ramfs_entry_t*)node->ptr;
    if (!entry) {
        return 0;
    }
    
    // Espandi il buffer se necessario
    if (offset + size > entry->size) {
        uint8_t* new_data = (uint8_t*)krealloc(entry->data, offset + size);
        if (!new_data) {
            return 0;
        }
        entry->data = new_data;
        entry->size = offset + size;
        node->length = entry->size;
    }
    
    memcpy(entry->data + offset, buffer, size);
    return size;
}

void ramfs_open(vfs_node_t* node) {
    // Nothing to do for ramfs
    (void)node;
}

void ramfs_close(vfs_node_t* node) {
    // Nothing to do for ramfs
    (void)node;
}

vfs_node_t* ramfs_readdir(vfs_node_t* node, uint32_t index) {
    ramfs_entry_t* dir_entry = (ramfs_entry_t*)node->ptr;
    if (!dir_entry || !(node->flags & VFS_DIRECTORY)) {
        return NULL;
    }
    
    ramfs_entry_t* child = dir_entry->children;
    uint32_t i = 0;
    
    while (child && i < index) {
        child = child->next;
        i++;
    }
    
    return child ? child->node : NULL;
}

vfs_node_t* ramfs_finddir(vfs_node_t* node, char* name) {
    ramfs_entry_t* dir_entry = (ramfs_entry_t*)node->ptr;
    if (!dir_entry || !(node->flags & VFS_DIRECTORY)) {
        return NULL;
    }
    
    ramfs_entry_t* child = dir_entry->children;
    
    while (child) {
        if (strcmp(child->node->name, name) == 0) {
            return child->node;
        }
        child = child->next;
    }
    
    return NULL;
}

// Inizializza RAMFS
void init_ramfs() {
    // Crea la root directory
    ramfs_root_entry = (ramfs_entry_t*)kmalloc(sizeof(ramfs_entry_t));
    if (!ramfs_root_entry) {
        return;
    }
    
    memset(ramfs_root_entry, 0, sizeof(ramfs_entry_t));
    
    fs_root = vfs_create_node("/", VFS_DIRECTORY);
    if (!fs_root) {
        kfree(ramfs_root_entry);
        return;
    }
    
    fs_root->ptr = ramfs_root_entry;
    fs_root->read = ramfs_read;
    fs_root->write = ramfs_write;
    fs_root->open = ramfs_open;
    fs_root->close = ramfs_close;
    fs_root->readdir = ramfs_readdir;
    fs_root->finddir = ramfs_finddir;
    
    ramfs_root_entry->node = fs_root;
}

// Crea un file in RAMFS
vfs_node_t* ramfs_create_file(vfs_node_t* parent, const char* name) {
    if (!parent || !(parent->flags & VFS_DIRECTORY)) {
        return NULL;
    }
    
    ramfs_entry_t* parent_entry = (ramfs_entry_t*)parent->ptr;
    
    // Crea la nuova entry
    ramfs_entry_t* new_entry = (ramfs_entry_t*)kmalloc(sizeof(ramfs_entry_t));
    if (!new_entry) {
        return NULL;
    }
    memset(new_entry, 0, sizeof(ramfs_entry_t));
    
    // Crea il nodo VFS
    vfs_node_t* new_node = vfs_create_node(name, VFS_FILE);
    if (!new_node) {
        kfree(new_entry);
        return NULL;
    }
    
    new_node->ptr = new_entry;
    new_node->read = ramfs_read;
    new_node->write = ramfs_write;
    new_node->open = ramfs_open;
    new_node->close = ramfs_close;
    
    new_entry->node = new_node;
    
    // Aggiungi alla lista dei children del parent
    new_entry->next = parent_entry->children;
    parent_entry->children = new_entry;
    
    return new_node;
}

// Crea una directory in RAMFS
vfs_node_t* ramfs_create_dir(vfs_node_t* parent, const char* name) {
    if (!parent || !(parent->flags & VFS_DIRECTORY)) {
        return NULL;
    }
    
    ramfs_entry_t* parent_entry = (ramfs_entry_t*)parent->ptr;
    
    // Crea la nuova entry
    ramfs_entry_t* new_entry = (ramfs_entry_t*)kmalloc(sizeof(ramfs_entry_t));
    if (!new_entry) {
        return NULL;
    }
    memset(new_entry, 0, sizeof(ramfs_entry_t));
    
    // Crea il nodo VFS
    vfs_node_t* new_node = vfs_create_node(name, VFS_DIRECTORY);
    if (!new_node) {
        kfree(new_entry);
        return NULL;
    }
    
    new_node->ptr = new_entry;
    new_node->read = ramfs_read;
    new_node->write = ramfs_write;
    new_node->open = ramfs_open;
    new_node->close = ramfs_close;
    new_node->readdir = ramfs_readdir;
    new_node->finddir = ramfs_finddir;
    
    new_entry->node = new_node;
    
    // Aggiungi alla lista dei children del parent
    new_entry->next = parent_entry->children;
    parent_entry->children = new_entry;
    
    return new_node;
}