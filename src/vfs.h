#ifndef VFS_H
#define VFS_H

#include <stdint.h>

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
    
    struct vfs_node* ptr; // Per mountpoint o symlink
} vfs_node_t;

extern vfs_node_t* fs_root;

uint32_t vfs_read(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t vfs_write(vfs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void vfs_open(vfs_node_t* node, uint8_t read, uint8_t write);
void vfs_close(vfs_node_t* node);
vfs_node_t* vfs_readdir(vfs_node_t* node, uint32_t index);
vfs_node_t* vfs_finddir(vfs_node_t* node, char* name);

#endif
