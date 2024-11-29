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
    FILE* ifp = NULL, * ofp = NULL;
    uintptr_t access_addr;
    char access_type;
    int accessed_data;

    init_memory_content();
    init_cache_content();

    ifp = fopen("access_input.txt", "r");
    if (ifp == NULL) {
        printf("Can't open input file\n");
        return -1;
    }
    ofp = fopen("access_output.txt", "w");
    if (ofp == NULL) {
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }

    // Write header for accessed data
    fprintf(ofp, "[Accessed Data]\n");

    /* Read access_input.txt and invoke retrieve_data() */
    while (fscanf(ifp, "%lu %c", &access_addr, &access_type) != EOF) {
        accessed_data = retrieve_data((void*)access_addr, access_type, ofp);
    }

    /* Calculate and print hit ratio and bandwidth */
    double hit_ratio = (double)num_cache_hits / (num_cache_hits + num_cache_misses);
    double bandwidth = (double)num_bytes / num_access_cycles;

    fprintf(ofp, "-----------------------------------------\n");
    fprintf(ofp, "[Direct mapped cache performance]\n");
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", hit_ratio, num_cache_hits, num_cache_hits + num_cache_misses);
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n", bandwidth, num_bytes, num_access_cycles);

    fclose(ifp);
    fclose(ofp);

    return 0;
}
