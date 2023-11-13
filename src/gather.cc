#include "shared.hh"
#include "util.hh"
#include "params.hh"
#include <iostream>
#include <fstream>

void print_results(uint64_t* results, long int size);

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    

    const long int num_iterations = BUFFER_SIZE_MB / ROW_SIZE;
    uint64_t bank_latency[num_iterations - 1] = {0};
    char *base = (char *)allocated_mem;

    for (int i = 1; i < num_iterations; i++) {
        uint64_t time = measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        bank_latency[i - 1] = time;
    }

    FILE *fp = fopen("data.tsv", "w");
    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }
    fprintf(fp, "Address\tTime\n");
    int possible_index = 0;
    for (int i = 0; i < num_iterations - 1; i++) {
        fprintf(fp, "%d\t%ld\n", i + 1, bank_latency[i]);
        if (bank_latency[i] > 255) {
            possible_index = i;
        }
    }
    fclose(fp);

    printf("Base: %ld\n", (uint64_t)base);
    printf("Possible same bank different col: %ld\n", (uint64_t)(base + possible_index * ROW_SIZE));

    print_results(bank_latency, num_iterations - 1);    

    // get the index where either top decile or max appears and use that to create a second address
    // print the 2 addresses and compare the bits and check if it matches the bank address mapping typically used
    // once that is figured out you can construct any 2 addresses given address n that lie in the same bank but are n + 1 and n - 1 rows away
    // then hammer
}

// Supporting functions for printing results in different formats
// Function "compare" is used in the priting functions and you do not need it
int compare(const void *p1, const void *p2) {
  uint64_t u1 = *(uint64_t *)p1;
  uint64_t u2 = *(uint64_t *)p2;

  return (int)u1 - (int)u2;
}

void print_results(uint64_t* results, long int size) {
    qsort(results, size, sizeof(uint64_t), compare);
    printf("Minimum      : %ld\n", results[0]);
    printf("Bottom decile: %ld\n", results[size/10]);
    printf("Median       : %ld\n", results[size/2]);
    printf("Top decile   : %ld\n", results[(size * 9)/10]);
    printf("Maximum      : %ld\n", results[size-1]);
}