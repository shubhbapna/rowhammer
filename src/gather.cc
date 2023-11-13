#include "shared.hh"
#include "util.hh"
#include "params.hh"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);

    if (getuid()) {
        printf("ERROR: Root permissions required!\n");
        exit(1);
    }

    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    

    const long int num_iterations = BUFFER_SIZE_MB / ROW_SIZE;
    uint64_t bank_latency[num_iterations] = {0};
    char *base = (char *)allocated_mem;

    for (int i = 1; i <= num_iterations; i++) {
        uint64_t time = measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        bank_latency[i - 1] = time;
    }

    for (int i = 0; i < num_iterations; i++) {
        printf("%d\t%ld", i + 1, bank_latency[i]);
    }
    
}