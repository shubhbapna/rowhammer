#include "../shared.hh"
#include "../util.hh"
#include "../params.hh"
#include <cstdlib>

int get_max_bucket(uint64_t *lat) {
    int max = 0;
    int max_i = 0;
    for (int i = 0; i < NUM_LAT_BUCKETS + 1; i++) {
        if (lat[i] > max) {
            max = lat[i];
            max_i = i;
        }
    }
    return max_i;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    // 90% of phy mem
    uint64_t mem_size = 1.8 * BUFFER_SIZE;
    allocated_mem = allocate_pages(mem_size);
    setup_PPN_VPN_map(allocated_mem, mem_size);

    const int num_bits = 32;

    uint64_t rows[num_bits] = {0};
    for (int x = 0; x < num_bits; x++) {
        uint64_t* bit_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
        for (int s = 0; s < SAMPLES; s++) {
            uint64_t addr, addr0, addr1;
            int tries = 1000;
    //setup_PPN_VPN_map(allocated_mem, mem_size);
            while (tries-- > 0) {
                addr = virt_to_phys((uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % (mem_size / PAGE_SIZE))));
                addr0 = phys_to_virt(addr ^ (addr & (1 << x)));
                addr1 = phys_to_virt(addr | (1  << x));
                if (addr != 0 && addr0 != 0 && addr1 != 0) break;
            }
	    printf("\n");
            if (tries <= 0) continue;
            print_result(addr, addr0, addr1, 0);
	    uint64_t time = two_address_access_latency(addr0, addr1);
            bit_lat_histogram[time / BUCKET_LAT_STEP]++;
        }
        
        rows[x] = get_max_bucket(bit_lat_histogram) * BUCKET_LAT_STEP + BUCKET_LAT_STEP / 2;
        free(bit_lat_histogram);
    }

    for (int i = 0; i < num_bits; i++) {
        printf("Bit %d: %10ld\n", i, rows[i]);        
    }
}
