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



/* Including headers for the program */
#include <stdio.h>          // Standard Input Output Library
#include <stdint.h>         // Standard Integer Library (uintptr_t type) - integer types are defined
#include "cache_impl.h"     // User-defined header (global variables & abstract functions declared)



/* Variables : Cache */
int num_cache_hits = 0;     // Variable storing the number of cache hit  
int num_cache_misses = 0;   // Variable storing the number of cache miss

int num_bytes = 0;          // Variable storing the total number of bytes accessed, required for the calculation of bandwidth
int num_access_cycles = 0;  // Variable storing the accumulation of CPU cycle (CACHE_ACCESS_CYCLE + MEMORY_ACCESS_CYCLE)

int global_timestamp = 0;   // Variable storing the order of cache access for each block, required for LRU eviction in case of conflict misses



/* Function 01
 * Searching data from the given byte address in the access_input.txt file.
 * If the data is already in cache, num_cache_hit += 1
 * Else, fetch it from the memory, MEMORY_ACCESS_CYCLE(100)
 */
int retrieve_data(void* addr, char data_type, FILE* ofp) {
    int value_returned = -1;                // Initialisation of data to be returned, default value is -1
    uintptr_t address = (uintptr_t)addr;    // Typecasting addr, as pointer addr has to be converted into integer in order to calculate set, tag, block offset, etc.

    /* Invoke check_cache_data_hit() */
    int cache_hit = check_cache_data_hit(addr, data_type);

    if (cache_hit != -1) {
        /* Case : hit, data returned */
        value_returned = cache_hit; // Data found (hit) is stored in value_returned variable 
        num_cache_hits++;           // Increment the number of cache hit by 1
    } else {
        /* Case : miss, -1 returned */
        value_returned = access_memory(addr, data_type);    // Data not present in cache, thus invoke access_memory() to fetch data directly from the memory and load it on cache 
        num_cache_misses++;                                 // Increment the number of cache miss by 1 
    }

    /* Write access result to access_output.txt */
    if (data_type == 'b') {
        // Data Type Case : byte(b)
        fprintf(ofp, "%lu\t%c\t0x%x\n", (unsigned long)address, data_type, value_returned & 0xFF);
    } else if (data_type == 'h') {
        // Data Type Case : halfword(h)
        fprintf(ofp, "%lu\t%c\t0x%x\n", (unsigned long)address, data_type, value_returned & 0xFFFF);
    } else if (data_type == 'w') {
        // Data Type Case : word(w)
        fprintf(ofp, "%lu\t%c\t0x%x\n", (unsigned long)address, data_type, value_returned);
    }

    /* Accessed data of the specified address is sent back to the caller(main.c) */
    return value_returned;
}





/* Function 02
 * Main function, called first when the program is ran.
 */
int main(void) {
    FILE* ifp = NULL, * ofp = NULL; // Declaration of file pointers for input.txt and output.txt
    uintptr_t access_addr;          // Variable storing the byte address to be accessed from the first column of access_input.txt
    char access_type;               // Variable storing the data type(b, h, w) from the second column of the access_input.txt
    int accessed_data;              // Data to retrieve first from cache, and then from memory 


    /* initialize memory and cache by invoking init_memory_content() and init_cache_content() */ 
    init_memory_content();  // Initialisation of memory
    init_cache_content();   // Initialisation of cache

    /* Opening access_input.txt file as reading mode */
    ifp = fopen("access_input.txt", "r");
    if (ifp == NULL) {
        // In case of error opening access_input.txt file, print error message on terminal and terminate the program
        printf("Can't open input file\n");
        return -1;
    }
    /* Opening access_output.txt file as writing mode */
    ofp = fopen("access_output.txt", "w");
    if (ofp == NULL) {
        // In case of error opening access_output.txt file, print error message on terminal and terminate the program
        printf("Can't open output file\n");
        fclose(ifp);
        return -1;
    }

    /* Output Format : Writing header for the accessed data */
    fprintf(ofp, "[Accessed Data]\n");

    /* Invoke retrieve_data()
     * Read each line and fetch the data in given form (address, type)
     */
    while (fscanf(ifp, "%lu %c", &access_addr, &access_type) != EOF) {
        // Search data based on the address and type
        accessed_data = retrieve_data((void*)access_addr, access_type, ofp);
        // Invoke print_cache_entries() to print current cache status on terminal for the purpose of debugging
        print_cache_entries();     
        printf("\n");   // newline for readability
    }

    /* Calculate hit ratio and bandwidth */
    double hit_ratio = (double)num_cache_hits / (num_cache_hits + num_cache_misses);
    double bandwidth = (double)num_bytes / num_access_cycles;

    fprintf(ofp, "-----------------------------------------\n");

    /* Change the header of accessed_output.txt file based on DEFAULT_CACHE_ASSOC */
    if (DEFAULT_CACHE_ASSOC == 1) {
        // Case of n = 1 - Direct Mapped Cache
        fprintf(ofp, "[Direct mapped cache performance]\n"); 
    } else if (DEFAULT_CACHE_ASSOC == 2) {
        // Case of n = 2 - 2-way associative Cache
        fprintf(ofp, "[2-way set associative cache performance]\n");
    } else if (DEFAULT_CACHE_ASSOC == 4) {
        // Case of n = 4 - fully associative Cache (To extend the program, the comparative can be set DEFAULT_CACHE_SIZE_BYTE/DEFAULT_CACHE_BLOCK_SIZE_BYTE)
        fprintf(ofp, "[Fully associative cache performance]\n");
    } else {
        fprintf(ofp, "[%d-way set associative cache performance]\n", DEFAULT_CACHE_ASSOC);
    }

    /* Print hit ratio and bandwidth of the given status of cache */
    fprintf(ofp, "Hit ratio = %.2f (%d/%d)\n", hit_ratio, num_cache_hits, num_cache_hits + num_cache_misses);
    fprintf(ofp, "Bandwidth = %.2f (%d/%d)\n", bandwidth, num_bytes, num_access_cycles);

    /* close files */
    fclose(ifp);
    fclose(ofp);

    /* Termination of program */
    return 0;
}
