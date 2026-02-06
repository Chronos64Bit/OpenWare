/**
 * OpenWare OS - Virtual File System (VFS)
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef VFS_H
#define VFS_H

#include "../include/types.h"

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04

typedef struct dirent {
    char name[128];
    uint32_t inode;
} dirent_t;

typedef struct fs_node {
    char name[128];
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint32_t impl; /* implementation defined number (e.g., start cluster) */
    
    /* Function pointers for operations */
    uint32_t (*read)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
    uint32_t (*write)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
    void (*open)(struct fs_node*);
    void (*close)(struct fs_node*);
    struct dirent* (*readdir)(struct fs_node*, uint32_t);
    struct fs_node* (*finddir)(struct fs_node*, char* name);
} fs_node_t;

/* Global Root Node */
extern fs_node_t* fs_root;

/* Standard VFS calls */
uint32_t vfs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t vfs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void vfs_open(fs_node_t* node);
void vfs_close(fs_node_t* node);
dirent_t* vfs_readdir(fs_node_t* node, uint32_t index);
fs_node_t* vfs_finddir(fs_node_t* node, char* name);

#endif
