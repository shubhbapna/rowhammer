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
        uint64_t time = 0;
        for (int j = 0; j < SAMPLES; j++) {
            time += measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        }
        bank_latency[i - 1] = time / SAMPLES;
    }

    FILE *fp = fopen("data.tsv", "w");
    if (fp == NULL) {
        perror("Error opening file");
        exit(1);
    }
    fprintf(fp, "Address\tTime\n");
    int different_rows = 0;
    int same_rows = 0;
    for (int i = 0; i < num_iterations - 1; i++) {
        fprintf(fp, "%d\t%ld\n", i + 1, bank_latency[i]);
        if (bank_latency[i] > 255 && bank_latency[i] < 450) {
            different_rows = i;
        }
        if (bank_latency[i] < 30) {
            same_rows = i;
        }
    }
    fclose(fp);

    printf("Base: %ld\n", virt_to_phys((uint64_t)base));
    printf("Possible same bank different row (index: %d): %ld\n", different_rows, virt_to_phys((uint64_t)(base + different_rows * ROW_SIZE)));
    printf("Possible same row (index: %d): %ld\n", same_rows, virt_to_phys((uint64_t)(base + same_rows * ROW_SIZE)));

    print_results(bank_latency, num_iterations - 1);    

    // get the index where either top decile or max appears and use that to create a second address
    // print the 2 addresses (convert to physical address) and compare the bits and check if it matches the bank address mapping typically used
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