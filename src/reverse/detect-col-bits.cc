#include "../shared.hh"
#include "../util.hh"
#include "../params.hh"
#include <cstdlib>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    // 90% of phy mem
    allocated_mem = allocate_pages(1.8 * BUFFER_SIZE_MB);
    setup_PPN_VPN_map(allocated_mem, 1.8 * BUFFER_SIZE_MB);

    uint64_t addr = virt_to_phys((uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % 1000)));
    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    uint64_t rows[13] = {0};
    for (int x = 0; x < 13; x++) {
        uint64_t addr0 = phys_to_virt(addr ^ (addr & (1 << x)));
        uint64_t addr1 = phys_to_virt(addr | (1  << x));
        uint64_t time = 0;
        for (int k = 0; k < SAMPLES; k++) {
            time += measure_bank_latency(addr0, addr1);
        }
        time = time / SAMPLES;
        rows[x] = time;
    }

    printf("No conflict found in bits: ");
    for (int i = 0; i < 13; i++) {
        if (rows[i] < ROW_BUFFER_CONFLICT_LATENCY) {
            printf("%d ", i);
        } else {
            printf("\nConflict found in bit %d", i);
        }
    }
}