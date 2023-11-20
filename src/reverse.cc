#include "shared.hh"
#include "util.hh"
#include "params.hh"
#include <set>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    const long int num_iterations = BUFFER_SIZE_MB / ROW_SIZE;
    char *base = (char *)allocated_mem;
    std::set<int> conflicts;

    for (int i = 1; i < num_iterations; i++) {
        uint64_t time = 0;
        for (int trials = 0; trials < SAMPLES; trials++) {
            time += measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        }
        time = time / SAMPLES;
        // keeping less than 500 to avoid noise
        if (ROW_BUFFER_CONFLICT_LATENCY <= time < 500) {
            conflicts.insert(i);
        }
    }
    int flag = 0;
    int index_a, index_b;
    for (int a: conflicts) {
        uint64_t two_address_access = 0;
        for (int trials = 0; trials < SAMPLES; trials++){
            two_address_access += measure_bank_latency((uint64_t)base, (uint64_t)(base + a * ROW_SIZE));
        }
        two_address_access = two_address_access / SAMPLES;
        
        for (int b: conflicts) {
            if (a == b) break;
            uint64_t three_address_access = 0;
            for (int trials = 0; trials < SAMPLES; trials++){
                three_address_access += measure_bank_latency3((uint64_t)base, (uint64_t)(base + a * ROW_SIZE), (uint64_t)(base + b * ROW_SIZE));
            }
            three_address_access = three_address_access / SAMPLES;
            if (three_address_access > two_address_access + 10); {
                flag = 1;
                index_b = b;
                break;
            }
        }

        if (flag) {
            index_a = a;
            break;
        }
    }

    uint64_t x = virt_to_phys((uint64_t) base);
    uint64_t a = virt_to_phys((uint64_t) (base + index_a * ROW_SIZE));
    uint64_t b = virt_to_phys((uint64_t) (base + index_b * ROW_SIZE));
    printf("X: %ld (phys)\t%ld (virt)\n", x, (uint64_t) base);
    printf("A: %ld (phys)\t%d (virt)\n", a, index_a);
    printf("B: %ld (phys)\t%d (virt)\n", b, index_b);
}