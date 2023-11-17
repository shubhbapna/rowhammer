#include "shared.hh"
#include "util.hh"
#include "params.hh"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    const long int num_iterations = BUFFER_SIZE_MB / ROW_SIZE;
    char *base = (char *)allocated_mem;

    int index_y;
    for (int i = 1; i < num_iterations; i++) {
        uint64_t time = 0;
        for (int k = 0; k < SAMPLES; k++) {
            time += measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        }
        time = time / SAMPLES;
        if (ROW_BUFFER_CONFLICT_LATENCY_MIN < time) {
            index_y = i;
            break;
        }
    }

    uint64_t x = virt_to_phys((uint64_t) base);
    uint64_t y = virt_to_phys((uint64_t) (base + index_y * ROW_SIZE));
    printf("X: %ld, Y: %ld, ", x, y);

    int index_a;
    for (int i = 1; i < num_iterations; i++) {
        if (i == index_y) continue;
        
        uint64_t time = 0;
        for (int k = 0; k < SAMPLES; k++) {
            time += measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        }
        time = time / SAMPLES;
        
        if (time < ROW_BUFFER_HIT_LATENCY) {
            time = 0;
            for (int k = 0; k < SAMPLES; k++) {
                time += measure_bank_latency((uint64_t)(base + index_y * ROW_SIZE), (uint64_t)(base + i * ROW_SIZE));
            }
            time = time / SAMPLES;
            if (ROW_BUFFER_CONFLICT_LATENCY_MIN < time) {
                index_a = i;
                break;
            }    
        }
    }

    uint64_t a = virt_to_phys((uint64_t) (base + index_a * ROW_SIZE));
    printf("A: %ld\n", a);
}