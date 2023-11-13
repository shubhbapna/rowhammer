#include "shared.hh"
#include "util.hh"
#include "params.hh"

void print_results(uint64_t* same, uint64_t* diff);

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

    print_results(bank_latency, num_iterations - 1);    
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
    printf("Minimum      : %5ld\n", results[0]);
    printf("Bottom decile: %5ld\n", results[size/10]);
    printf("Median       : %5ld\n", results[size/2]);
    printf("Top decile   : %5ld\n", results[(size * 9)/10]);
    printf("Maximum      : %5ld\n", results[size-1]);
}