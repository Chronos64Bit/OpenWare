/**
 * OpenWare OS - Memory Management (Heap)
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// Define heap size (1MB for now)
#define HEAP_START 0x1000000  // Start allocation at 16MB mark
#define HEAP_SIZE  0x800000   // 8MB Heap


typedef struct alloc_block {
    size_t size;
    bool free;
    struct alloc_block* next;
} alloc_block_t;

void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void* kcalloc(size_t num, size_t size);
void* kmemcpy(void* dest, const void* src, size_t n);
void* kmemset(void* s, int c, size_t n);

#endif
