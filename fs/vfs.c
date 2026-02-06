/**
 * OpenWare OS - Virtual File System (VFS) Implementation
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#include "vfs.h"

fs_node_t* fs_root = 0;

uint32_t vfs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->read != 0)
        return node->read(node, offset, size, buffer);
    return 0;
}

uint32_t vfs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node->write != 0)
        return node->write(node, offset, size, buffer);
    return 0;
}

void vfs_open(fs_node_t* node) {
    if (node->open != 0)
        node->open(node);
}

void vfs_close(fs_node_t* node) {
    if (node->close != 0)
        node->close(node);
}

dirent_t* vfs_readdir(fs_node_t* node, uint32_t index) {
    if (node->readdir != 0)
        return node->readdir(node, index);
    return 0;
}

fs_node_t* vfs_finddir(fs_node_t* node, char* name) {
    if (node->finddir != 0)
        return node->finddir(node, name);
    return 0;
}
