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

#include <stdio.h>
#include <string.h>
#include <stdint.h>  // 변경: stdint.h를 사용하여 정수형 타입을 정의
#include "cache_impl.h"

extern int num_cache_hits;
extern int num_cache_misses;

extern int num_bytes;
extern int num_access_cycles;

extern int global_timestamp;

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

int check_cache_data_hit(void *addr, char type) {
    uintptr_t address = (uintptr_t)addr;
    int block_offset = address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int set_index = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE;
    int tag = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE;

    // Search the cache set for a valid entry with matching tag
    for (int i = 0; i < DEFAULT_CACHE_ASSOC; i++) {
        cache_entry_t *pEntry = &cache_array[set_index][i];
        if (pEntry->valid && pEntry->tag == tag) {
            // Cache hit
            num_access_cycles += CACHE_ACCESS_CYCLE;
            pEntry->timestamp = global_timestamp++;
            
            // Extract data based on the requested type
            int data = 0;
            if (type == 'b') {
                data = pEntry->data[block_offset];
                num_bytes += 1;
            } else if (type == 'h') {
                data = *((short *)(pEntry->data + block_offset));
                num_bytes += 2;
            } else if (type == 'w') {
                data = *((int *)(pEntry->data + block_offset));
                num_bytes += 4;
            }
            return data;
        }
    }

    // Cache miss
    return -1;
}

int find_entry_index_in_set(int cache_index) {
    int entry_index;

    // Check if there exists any empty cache space by checking 'valid'
    for (entry_index = 0; entry_index < DEFAULT_CACHE_ASSOC; entry_index++) {
        if (cache_array[cache_index][entry_index].valid == 0) {
            return entry_index; // If there is an empty space, return the index of the empty cache entry
        }
    }
    
    // Otherwise, search over all entries to find the least recently used entry by checking 'timestamp'
    int lru_index = 0;
    int min_timestamp = cache_array[cache_index][0].timestamp;
    for (entry_index = 1; entry_index < DEFAULT_CACHE_ASSOC; entry_index++) {
        if (cache_array[cache_index][entry_index].timestamp < min_timestamp) {
            min_timestamp = cache_array[cache_index][entry_index].timestamp;
            lru_index = entry_index;
        }
    }

    return lru_index; // return index of LRU entry
}

int access_memory(void *addr, char type) {
    uintptr_t address = (uintptr_t)addr;
    int word_address = address / WORD_SIZE_BYTE;
    int block_offset = address % DEFAULT_CACHE_BLOCK_SIZE_BYTE;
    int set_index = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) % CACHE_SET_SIZE;
    int tag = (address / DEFAULT_CACHE_BLOCK_SIZE_BYTE) / CACHE_SET_SIZE;

    int entry_index = find_entry_index_in_set(set_index);

    cache_entry_t *pEntry = &cache_array[set_index][entry_index];
    pEntry->valid = 1;
    pEntry->tag = tag;
    pEntry->timestamp = global_timestamp++;

    // Copy data from memory to cache block
    int word_start_address = (word_address / (DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE)) * (DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE);
    for (int i = 0; i < (DEFAULT_CACHE_BLOCK_SIZE_BYTE / WORD_SIZE_BYTE); i++) {
        int memory_index = word_start_address + i;
        memcpy(pEntry->data + (i * WORD_SIZE_BYTE), &memory_array[memory_index], WORD_SIZE_BYTE);
    }

    num_access_cycles += MEMORY_ACCESS_CYCLE;

    // Extract data based on the requested type
    int data = 0;
    if (type == 'b') {
        data = pEntry->data[block_offset];
        num_bytes += 1;
    } else if (type == 'h') {
        data = *((short *)(pEntry->data + block_offset));
        num_bytes += 2;
    } else if (type == 'w') {
        data = *((int *)(pEntry->data + block_offset));
        num_bytes += 4;
    }

    return data;
}
