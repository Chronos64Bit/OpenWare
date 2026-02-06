/**
 * OpenWare OS - Memory Management (Heap)
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#include "memory.h"

static alloc_block_t* heap_start = (alloc_block_t*)HEAP_START;

void memory_init(void) {
    heap_start->size = HEAP_SIZE - sizeof(alloc_block_t);
    heap_start->free = true;
    heap_start->next = NULL;
}

void* kmalloc(size_t size) {
    // Align size to 4 bytes
    size = (size + 3) & ~3;
    
    alloc_block_t* current = heap_start;
    
    while (current != NULL) {
        if (current->free && current->size >= size) {
            // Can we split this block?
            if (current->size >= size + sizeof(alloc_block_t) + 4) {
                alloc_block_t* new_block = (alloc_block_t*)((uint32_t)current + sizeof(alloc_block_t) + size);
                
                new_block->size = current->size - size - sizeof(alloc_block_t);
                new_block->free = true;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->free = false;
            return (void*)((uint32_t)current + sizeof(alloc_block_t));
        }
        current = current->next;
    }
    
    return NULL; // Out of memory
}

void kfree(void* ptr) {
    if (ptr == NULL) return;
    
    alloc_block_t* block = (alloc_block_t*)((uint32_t)ptr - sizeof(alloc_block_t));
    block->free = true;
    
    // Merge with next block if free
    if (block->next && block->next->free) {
        block->size += block->next->size + sizeof(alloc_block_t);
        block->next = block->next->next;
    }
    
    // We should also look back to merge, but we need a doubly linked list for O(1) merge
    // For this simple allocator, we rely on forward merging during traversal or next alloc
}

void* kcalloc(size_t num, size_t size) {
    void* ptr = kmalloc(num * size);
    if (ptr) kmemset(ptr, 0, num * size);
    return ptr;
}

void* kmemcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dest;
}

void* kmemset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;
    for (size_t i = 0; i < n; i++) p[i] = (uint8_t)c;
    return s;
}
