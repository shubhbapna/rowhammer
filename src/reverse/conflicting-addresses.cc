#include "../shared.hh"
#include "../util.hh"
#include "../params.hh"
#include <set>
#include <cstdlib>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE);

    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    const long int num_iterations = BUFFER_SIZE / ROW_SIZE;
    char *base = (char *)allocated_mem;
    std::set<int> conflicts;

    for (int i = 1; i < num_iterations; i++) {
        uint64_t time = two_address_access_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        // keeping less than 500 to avoid noise
        if (ROW_BUFFER_CONFLICT_LATENCY <= time && time < 600) {
            conflicts.insert(i);
        }
    }
    int flag = 0;
    int index_a, index_b;
    for (int a: conflicts) {
        uint64_t two_address_access_time = two_address_access_latency((uint64_t)base, (uint64_t)(base + a * ROW_SIZE));
        
        for (int b: conflicts) {
            if (a == b) break;
            uint64_t three_address_access_time = three_address_access_latency((uint64_t)base, (uint64_t)(base + a * ROW_SIZE), (uint64_t)(base + b * ROW_SIZE));
            if (three_address_access_time > two_address_access_time + 120) {
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
    printf("X: %s\t%ld (phys)\t%ld (virt)\n", int_to_binary(x, 33), x, (uint64_t) base);
    printf("A: %s\t%ld (phys)\t%d (virt)\n", int_to_binary(a, 33), a, index_a);
    printf("B: %s\t%ld (phys)\t%d (virt)\n", int_to_binary(b, 33), b, index_b);
}