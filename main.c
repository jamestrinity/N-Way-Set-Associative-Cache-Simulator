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
#include "cache_impl.h"

int num_cache_hits = 0;
int num_cache_misses = 0;

int num_bytes = 0;
int num_access_cycles = 0;

int global_timestamp = 0;

int retrieve_data(void *addr, char data_type) {
    int value_returned = 0; /* accessed data */

    /* Invoke check_cache_data_hit() */
    if (check_cache_data_hit(addr, data_type)) {
        num_cache_hits++;
        return value_returned; 
    } else {
        /* In case of the cache miss event, access the main memory by invoking access_memory() */
        value_returned = access_memory(addr, data_type);
        num_cache_misses++;
    }

    /* If there is no data neither in cache nor memory, return -1 else return data */
    if (value_returned == 0) {
        return -1;
    }

    return value_returned;    
}

int main(void) {
    FILE * ifp = NULL, *ofp = NULL; // file pointers
    unsigned long int access_addr; /* byte address (located at 1st column) in "access_input.txt" */
    char access_type; /* 'b'(byte), 'h'(halfword), or 'w'(word) (located at 2nd column) in "access_input.txt" */
    int accessed_data; /* This is the data that you want to retrieve first from cache, and then from memory */ 
    
    init_memory_content();/* initialize memory and cache by invoking init_memory_content() and init_cache_content() */ 
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

    
    /* read each line and get the data in given (address, type) by invoking retrieve_data() */
    while (fscanf(ifp, "%lx %c", &access_addr, &access_type) != EOF) {
        accessed_data = retrieve_data((void *)access_addr, access_type);
        fprintf(ofp, "Address: %lx, Type: %c, Data: %d\n", access_addr, access_type, accessed_data);
    }

    /* print hit ratio and bandwidth for each cache mechanism as regards to cache association size */
    double hit_ratio = (double)num_cache_hits / (num_cache_hits + num_cache_misses);
    double bandwidth = (double)num_bytes / num_access_cycles;

    fprintf(ofp, "Cache Hit Ratio: %.2f\n", hit_ratio);
    fprintf(ofp, "Bandwidth: %.2f bytes/cycle\n", bandwidth);


    /* close files */
    fclose(ifp);
    fclose(ofp);

    /* print the final cache entries by invoking print_cache_entries() */ 
    print_cache_entries();
    return 0;
}
