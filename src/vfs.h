#ifndef VFS_H
#define VFS_H

#include <stdint.h>

// Flags per i nodi VFS
#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02
#define VFS_CHARDEV   0x04
#define VFS_BLOCKDEV  0x08
#define VFS_PIPE      0x10
#define VFS_SYMLINK   0x20
#define VFS_MOUNTPOINT 0x40

typedef struct vfs_node {
    char name[128];
    uint32_t flags;     // Directory, file, etc
    uint32_t inode;
    uint32_t length;
    
    // Funzioni del filesystem
    uint32_t (*read)(struct vfs_node*, uint32_t offset, uint32_t size, uint8_t* buffer);
    uint32_t (*write)(struct vfs_node*, uint32_t offset, uint32_t size, uint8_t* buffer);
    void (*open)(struct vfs_node*);
    void (*close)(struct vfs_node*);
    struct vfs_node* (*readdir)(struct vfs_node*, uint32_t index);
    struct vfs_node* (*finddir)(struct vfs_node*, char* name);
    
    void* ptr; // Per implementazioni specifiche del filesystem
} vfs_node_t;

extern vfs_node_t* fs_root;

// Funzioni wrapper VFS
uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void vfs_open(vfs_node_t* node, uint8_t read, uint8_t write);
void vfs_close(vfs_node_t* node);
vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index);
vfs_node_t* vfs_finddir(vfs_node_t* node, char* name);

// Funzioni di gestione nodi
vfs_node_t* vfs_create_node(const char* name, uint32_t flags);
void vfs_free_node(vfs_node_t* node);

// RAMFS functions
void init_ramfs();
vfs_node_t* ramfs_create_file(vfs_node_t* parent, const char* name);
vfs_node_t* ramfs_create_dir(vfs_node_t* parent, const char* name);

#endif