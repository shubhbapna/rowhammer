#include "shared.hh"
#include "util.hh"
#include "params.hh"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    const long int num_iterations = BUFFER_SIZE_MB / ROW_SIZE;
    char *base = (char *)allocated_mem;
    
    uint64_t same_row_lat = 0;
    // warm the address up
    measure_bank_latency((uint64_t)base, (uint64_t)(base));
    for (int k = 0; k < SAMPLES; k++) {
        same_row_lat += measure_bank_latency((uint64_t)base, (uint64_t)(base));
    }
    same_row_lat = same_row_lat / SAMPLES;
    printf("Same row latency: %ld\n", same_row_lat);

    int index_y;
    for (int i = 1; i < num_iterations; i++) {
        uint64_t time = 0;
        // warm the address up to avoid page faults and TLB misses
        measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        for (int k = 0; k < SAMPLES; k++) {
            time += measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        }
        time = time / SAMPLES;
        if (ROW_BUFFER_CONFLICT_LATENCY < time) {
            index_y = i;
            break;
        }
    }

    uint64_t x = virt_to_phys((uint64_t) base);
    uint64_t y = virt_to_phys((uint64_t) (base + index_y * ROW_SIZE));
    printf("X: %ld (phys)\t%ld (virt)\n", x, (uint64_t) base);
    printf("Y: %ld (phys)\t%d (virt)\n", y, index_y);

    int index_a[5];
    int count = 0;
    for (int i = 1; i < num_iterations; i++) {
        if (i == index_y) continue;
        
        uint64_t time = 0;
        measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        for (int k = 0; k < SAMPLES; k++) {
            time += measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        }
        time = time / SAMPLES;
        
        if (time < ROW_BUFFER_HIT_LATENCY) {
            time = 0;
            measure_bank_latency((uint64_t)(base + index_y * ROW_SIZE), (uint64_t)(base + i * ROW_SIZE));
            for (int k = 0; k < SAMPLES; k++) {
                time += measure_bank_latency((uint64_t)(base + index_y * ROW_SIZE), (uint64_t)(base + i * ROW_SIZE));
            }
            time = time / SAMPLES;
            if (ROW_BUFFER_CONFLICT_LATENCY < time) {
                if (count >=5 ) break;
                index_a[count] = i;
                count++;
            }    
        }
    }

    for (int i = 0; i < count; i++) {
        uint64_t a = virt_to_phys((uint64_t) (base + index_a[i] * ROW_SIZE));
        printf("A: %ld (phy)\t%d (virt)\n", a, index_a[i]);
    }
}