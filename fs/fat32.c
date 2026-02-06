#include "fat32.h"
#include "../kernel/ata.h"
#include "../kernel/ramdisk.h"
#include "../include/memory.h"
#include "../kernel/vga.h"

/* Global FAT32 State */
static fat_bpb_t bpb;
static uint32_t fat_begin_lba;
static uint32_t cluster_begin_lba;
static uint32_t sectors_per_cluster;
static uint32_t root_cluster;
static dirent_t current_dirent;

/* Forward declarations */
static uint32_t fat32_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
static fs_node_t* fat32_finddir(fs_node_t* node, char* name);
static dirent_t* fat32_readdir(fs_node_t* node, uint32_t index);

/* Helper: Read a cluster */
static void fat32_read_cluster(uint32_t cluster, uint8_t* buffer) {
    uint32_t lba = cluster_begin_lba + (cluster - 2) * sectors_per_cluster;
    // Switch to Ramdisk for "One ISO" solution
    ramdisk_read(lba, sectors_per_cluster, buffer);
}

/* Helper: Convert filename 8.3 to normal string */
static void fat_to_str(char* dest, char* src) {
    int i, j;
    // Copy Name
    for (i = 0; i < 8 && src[i] != ' '; i++) dest[i] = src[i];
    
    // Extension
    if (src[8] != ' ') {
        dest[i++] = '.';
        for (j = 0; j < 3 && src[8+j] != ' '; j++) dest[i++] = src[8+j];
    }
    dest[i] = 0;
}

/* Initialize FAT32 */
void fat32_init(void) {
    uint8_t* buffer = kmalloc(512);
    
    /* Read Boot Sector from Ramdisk */
    ramdisk_read(0, 1, buffer);
    
    kmemcpy(&bpb, buffer, sizeof(fat_bpb_t));
    kfree(buffer);
    
    /* Verify Signature */
    if (bpb.boot_signature != 0x29 && bpb.boot_signature != 0x28) {
        vga_puts("[FAT32] Checking Ramdisk... "); 
        // If signature fails, we might warn, but proceed if it looks partially valid
    }
    
    /* Calculate Offsets */
    fat_begin_lba = bpb.reserved_sectors;
    cluster_begin_lba = bpb.reserved_sectors + (bpb.fats_count * bpb.sectors_per_fat_32);
    sectors_per_cluster = bpb.sectors_per_cluster;
    root_cluster = bpb.root_cluster;
    
    /* Setup Root Node */
    fs_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    kmemset(fs_root, 0, sizeof(fs_node_t));
    
    // Copy name "ROOT"
    fs_root->name[0] = '/'; fs_root->name[1] = 0;
    fs_root->flags = FS_DIRECTORY;
    fs_root->impl = root_cluster; /* Store start cluster in 'impl' */
    fs_root->read = 0;
    fs_root->write = 0;
    fs_root->open = 0;
    fs_root->close = 0;
    fs_root->readdir = fat32_readdir;
    fs_root->finddir = fat32_finddir;
}

/* Read Directory Entry at Index */
static dirent_t* fat32_readdir(fs_node_t* node, uint32_t index) {
    if ((node->flags & 0x7) != FS_DIRECTORY) return 0;
    
    uint32_t cluster = node->impl;
    uint32_t cluster_size = sectors_per_cluster * 512;
    uint8_t* buffer = kmalloc(cluster_size);
    
    /* Read Directory Cluster */
    fat32_read_cluster(cluster, buffer);
    
    fat_dir_entry_t* entry = (fat_dir_entry_t*)buffer;
    int entries_count = cluster_size / sizeof(fat_dir_entry_t);
    uint32_t valid_idx = 0;
    
    for (int i = 0; i < entries_count; i++) {
        if (entry[i].name[0] == 0x00) break; /* End of directory */
        if (entry[i].name[0] == 0xE5) continue; /* Deleted */
        
        // Skip Long File Name (LFN) entries for now (attr == 0x0F)
        if (entry[i].attr == FAT_ATTR_LFN) continue;

        if (valid_idx == index) {
            fat_to_str(current_dirent.name, entry[i].name);
            current_dirent.inode = i;
            kfree(buffer);
            return &current_dirent;
        }
        valid_idx++;
    }
    
    kfree(buffer);
    return 0;
}

/* Find file in directory */
static fs_node_t* fat32_finddir(fs_node_t* node, char* name) {
    if ((node->flags & 0x7) != FS_DIRECTORY) return 0;
    
    uint32_t cluster = node->impl;
    uint32_t cluster_size = sectors_per_cluster * 512;
    uint8_t* buffer = kmalloc(cluster_size);
    
    /* Read Directory Cluster */
    /* TODO: Follow cluster chain if directory spans multiple clusters */
    fat32_read_cluster(cluster, buffer);
    
    fat_dir_entry_t* entry = (fat_dir_entry_t*)buffer;
    int entries_count = cluster_size / sizeof(fat_dir_entry_t);
    
    for (int i = 0; i < entries_count; i++) {
        if (entry[i].name[0] == 0x00) break; /* End of directory */
        if (entry[i].name[0] == 0xE5) continue; /* Deleted */
        
        char filename[13];
        fat_to_str(filename, entry[i].name);
        
        // Very simple string compare (case sensitive for now)
        int match = 1;
        for(int k=0; filename[k] || name[k]; k++) {
            if(filename[k] != name[k]) { match=0; break; }
        }
        
        if (match) {
            fs_node_t* file_node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
            kmemset(file_node, 0, sizeof(fs_node_t));
            
            kmemcpy(file_node->name, filename, 13);
            file_node->inode = i; // Index in dir
            file_node->length = entry[i].size;
            file_node->impl = (entry[i].first_cluster_hi << 16) | entry[i].first_cluster_lo;
            file_node->flags = FS_FILE;
            if (entry[i].attr & FAT_ATTR_DIRECTORY) file_node->flags = FS_DIRECTORY;
            
            file_node->read = fat32_read;
            file_node->finddir = fat32_finddir;
            
            kfree(buffer);
            return file_node;
        }
    }
    
    kfree(buffer);
    return 0;
}

/* Read file content */
static uint32_t fat32_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    uint32_t cluster = node->impl;
    uint32_t cluster_size = sectors_per_cluster * 512;
    uint8_t* cl_buffer = kmalloc(cluster_size);
    
    /* Simple Read: Read first cluster only for now */
    /* TODO: Follow cluster chain */
    fat32_read_cluster(cluster, cl_buffer);
    
    /* Copy request size */
    if (size > cluster_size) size = cluster_size; // Limit to one cluster for now
    if (size > node->length) size = node->length;
    
    kmemcpy(buffer, cl_buffer + offset, size);
    
    kfree(cl_buffer);
    return size;
}
