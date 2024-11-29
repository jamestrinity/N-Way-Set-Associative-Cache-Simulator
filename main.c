/*
 * main.c
 *
 * 20493-01 Computer Architecture
 * Term Project on Implementation of Cache Mechanism
 *
 * Skeleton Code Prepared by Prof. HyungJune Lee
 * Nov 17, 2024
 *
 */

#include <stdio.h>
#include <stdint.h>
#include "cache_impl.h"

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

int global_timestamp = 0;

int retrieve_data(void* addr, char data_type, FILE* ofp) {
    int value_returned = -1; /* accessed data */
    uintptr_t address = (uintptr_t)addr;

    /* Invoke check_cache_data_hit() */
    int cache_hit = check_cache_data_hit(addr, data_type);

    if (cache_hit != -1) {
        /* Cache hit */
        value_returned = cache_hit;
        num_cache_hits++;
    } else {
        /* Cache miss */
        value_returned = access_memory(addr, data_type);
        num_cache_misses++;
    }

    // Write access result to file in the specified format
    if (data_type == 'b') {
        fprintf(ofp, "%lu\t%c\t0x%x\n", (unsigned long)address, data_type, value_returned & 0xFF);
    } else if (data_type == 'h') {
        fprintf(ofp, "%lu\t%c\t0x%x\n", (unsigned long)address, data_type, value_returned & 0xFFFF);
    } else if (data_type == 'w') {
        fprintf(ofp, "%lu\t%c\t0x%x\n", (unsigned long)address, data_type, value_returned);
    }

    return value_returned;
}

int main(void) {
    FILE* ifp = NULL, * ofp = NULL; // file pointers
    uintptr_t access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */ 


    /* initialize memory and cache by invoking init_memory_content() and init_cache_content() */ 
    init_memory_content();
    init_cache_content();

     /* open input file as reading mode */
    ifp = fopen("access_input.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    /* open output file as writing mode */
    ofp = fopen("access_output.txt", "w");
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }

    // Write header for accessed data
    fprintf(ofp, "[Accessed Data]\n");

    /* read each line and get the data in given (address, type) by invoking retrieve_data() */
    while (fscanf(ifp, "%lu %c", &access_addr, &access_type) != EOF) {
        accessed_data = retrieve_data((void*)access_addr, access_type, ofp);
<<<<<<< Updated upstream
        print_cache_entries();
        printf("count\n");
=======
        print_cache_entries();     /* print the final cache entries by invoking print_cache_entries() */ 
        printf("\n");
>>>>>>> Stashed changes
    }

    /* Calculate and print hit ratio and bandwidth */
    double hit_ratio = (double)num_cache_hits / (num_cache_hits + num_cache_misses);
    double bandwidth = (double)num_bytes / num_access_cycles;

    fprintf(ofp, "-----------------------------------------\n");

    // Write the cache performance header based on DEFAULT_CACHE_ASSOC
    if (DEFAULT_CACHE_ASSOC == 1) {
        fprintf(ofp, "[Direct mapped cache performance]\n"); 
    } else if (DEFAULT_CACHE_ASSOC == 2) {
        fprintf(ofp, "[2-way set associative cache performance]\n");
    } else if (DEFAULT_CACHE_ASSOC == 4) {
        fprintf(ofp, "[Fully associative cache performance]\n");
    } else {
        fprintf(ofp, "[%d-way set associative cache performance]\n", DEFAULT_CACHE_ASSOC);
    }

    
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", hit_ratio, num_cache_hits, num_cache_hits + num_cache_misses);
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n", bandwidth, num_bytes, num_access_cycles);

    /* close files */
    fclose(ifp);
    fclose(ofp);

    
    return 0;
}
