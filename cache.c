/*
 * cache.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implementation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 17, 2024
 *
 */



/* Including headers for the program */
#include <stdio.h>          // Standard Input Output Library
#include <stdint.h>         // Standard Integer Library (uintptr_t type) - integer types are defined
#include <string.h>         // String Library - memcpy function defined
#include "cache_impl.h"     // User-defined header (global variables & abstract functions declared)



/* extern variables : variables already defined in main.c, thus only allowing referencing*/
extern int num_cache_hits;      // Variable storing the number of cache hit  
extern int num_cache_misses;    // Variable storing the number of cache miss

extern int num_bytes;           // Variable storing the total number of bytes accessed, required for the calculation of bandwidth
extern int num_access_cycles;   // Variable storing the accumulation of CPU cycle (CACHE_ACCESS_CYCLE + MEMORY_ACCESS_CYCLE)

extern int global_timestamp;    // Variable storing the order of cache access for each block, required for LRU eviction in case of conflict misses
  
/* Declaration of arrays : 1) Cache entry 2) Memory */
cache_entry_t cache_array[CACHE_SET_SIZE][DEFAULT_CACHE_ASSOC];
int memory_array[DEFAULT_MEMORY_SIZE_WORD];



/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_memory_content() {
    unsigned char sample_upward[16] = {0x001, 0x012, 0x023, 0x034, 0x045, 0x056, 0x067, 0x078, 0x089, 0x09a, 0x0ab, 0x0bc, 0x0cd, 0x0de, 0x0ef};
    unsigned char sample_downward[16] = {0x0fe, 0x0ed, 0x0dc, 0x0cb, 0x0ba, 0x0a9, 0x098, 0x087, 0x076, 0x065, 0x054, 0x043, 0x032, 0x021, 0x010};
    int index, i=0, j=1, gap = 1;
    
    for (index=0; index < DEFAULT_MEMORY_SIZE_WORD; index++) {
        memory_array[index] = (sample_upward[i] << 24) | (sample_upward[j] << 16) | (sample_downward[i] << 8) | (sample_downward[j]);
        if (++i >= 16)
            i = 0;
        if (++j >= 16)
            j = 0;
        
        if (i == 0 && j == i+gap)
            j = i + (++gap);
            
        printf("mem[%d] = %#x\n", index, memory_array[index]);
    }
}   



/* DO NOT CHANGE THE FOLLOWING FUNCTION */
void init_cache_content() {
    int i, j;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        for (j=0; j < DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            pEntry->valid = 0;
            pEntry->tag = -1;
            pEntry->timestamp = 0;
        }
    }
}



/* DO NOT CHANGE THE FOLLOWING FUNCTION */
/* This function is a utility function to print all the cache entries. It will be useful for your debugging */
void print_cache_entries() {
    int i, j, k;
    
    for (i=0; i<CACHE_SET_SIZE; i++) {
        printf("[Set %d] ", i);
        for (j=0; j <DEFAULT_CACHE_ASSOC; j++) {
            cache_entry_t *pEntry = &cache_array[i][j];
            printf("V: %d Tag: %#x Time: %d Data: ", pEntry->valid, pEntry->tag, pEntry->timestamp);
            for (k=0; k<DEFAULT_CACHE_BLOCK_SIZE_BYTE; k++) {
                printf("%#x(%d) ", pEntry->data[k], k);
            }
            printf("\t");
        }
        printf("\n");
    }
}



/* Function 03
 * Check if the data is found in cache (hit)
 */
int check_cache_data_hit(void *addr, char type) {
    uintptr_t address = (uintptr_t)addr;                                        // Typecasting addr, as pointer addr has to be converted into integer for the following three calcuations
    int block_offset = address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;                 // Block offset is calculated using the address
    int set_index = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE; // Set index is calculated using the address
    int tag = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE;       // Tag is calculated using the address

    /* Searching through the cache set for a valid entry with a matching tag */
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
        cache_entry_t *pEntry = &cache_array[set_index][i];
        if (pEntry->valid && pEntry->tag == tag) {
            // Case : Cache Hit, tag found
            num_access_cycles += CACHE_ACCESS_CYCLE;    // Increment cache access cycle only (100 cycle saved from memory access)
            pEntry->timestamp = global_timestamp++;     // Increment global timestamp for the next cache entry
            
            // Fetch data based on the requested type
            int data = 0;
            if (type == 'b') {
                data = pEntry->data[block_offset];
                num_bytes += 1;     // byte(1) access
            } else if (type == 'h') {
                data = *((short *)(pEntry->data + block_offset));
                num_bytes += 2;     // halfword(2) access
            } else if (type == 'w') {
                data = *((int *)(pEntry->data + block_offset));
                num_bytes += 4;     // word(4) access
            }
            return data;            // Fetched data is returned to the caller
        }
    }

    // Case : Cache Miss, tag not found
    return -1;
}



/* Function 04
 * Find an entry index from the set to be replaced
 */
int find_entry_index_in_set(int cache_index) {
    int entry_index;    // Variable storing the index of entry
    int temp;           // Variable temporarily storing the timestamp of the moment

    /* Check if there exists any empty cache space by checking 'valid' */
    for (entry_index = 0; entry_index < DEFAULT_CACHE_ASSOC; entry_index++) {
        if (cache_array[cache_index][entry_index].valid == 0) {
            /* If there is an empty space, return the index of the empty cache entry */
            return entry_index; 
        }
    }
    
    /* Otherwise, search over all entries to find the least recently used entry by checking 'timestamp' */
    for (entry_index = 0; entry_index < DEFAULT_CACHE_ASSOC; entry_index++) {
        if (cache_array[cache_index][entry_index].timestamp <= global_timestamp) {
            temp = cache_array[cache_index][entry_index].timestamp;
            if (temp < cache_array[cache_index][entry_index].timestamp) {
                // Update the LRU entry index (The least recently used one replaced with new data)
                temp = cache_array[cache_index][entry_index].timestamp;
            }
        }
    }

    /* return index of LRU entry*/  
    entry_index = temp;
    return entry_index; 
}



/* Function 05
 * In case of cache miss, this function is called to fetch data directly from the memory.
 * Extra 100 cycle added to the total CPU cycle
 */
int access_memory(void *addr, char type) {                                       
    uintptr_t address = (uintptr_t)addr;                                        // Typecasting addr, as pointer addr has to be converted into integer for the following three calcuations
    int word_address = address / WORD_SIZE_BYTE;                                // Conversion of unit from byte address to word address
    int block_offset = address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;                 // Block offset is calculated using the address
    int set_index = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE; // Set index is calculated using the address
    int tag = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE;       // Tag is calculated using the address

    int entry_index = find_entry_index_in_set(set_index);                       // Invoke find_entry_index_in_set() to find an entry index to replace the old data with a newly fetched one

    cache_entry_t *pEntry = &cache_array[set_index][entry_index];
    pEntry->valid = 1;                          // Setting the validity of cache entry
    pEntry->tag = tag;                          // Setting tag of the block
    pEntry->timestamp = global_timestamp++;     // Update global timestamp

    /* Loop to copy data from memory to cache block */
    // Calculation of the start address of the block
    int word_start_address = (word_address / (DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE)) * (DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE);
    for (int i = 0; i < (DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE); i++) {
        int memory_index = word_start_address + i;
        // From the start address, copy memory block using memcpy function
        memcpy(pEntry->data + (i * WORD_SIZE_BYTE), &memory_array[memory_index], WORD_SIZE_BYTE);
    }

    // As a result of cache miss, extra 100 cycle is taken 
    num_access_cycles += MEMORY_ACCESS_CYCLE;

    // Fetch data based on the requested type
    int data = 0;
    if (type == 'b') {
        data = pEntry->data[block_offset];
        num_bytes += 1;     // byte(1) access
    } else if (type == 'h') {
        data = *((short *)(pEntry->data + block_offset));
        num_bytes += 2;     // halfword(2) access
    } else if (type == 'w') {
        data = *((int *)(pEntry->data + block_offset));
        num_bytes += 4;     // word(4) access
    }

    return data;            // Fetched data is returned to the caller
}
