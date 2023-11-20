#include "shared.hh"
#include "util.hh"
#include "params.hh"
#include <cstdlib>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    setup_PPN_VPN_map(allocated_mem, BUFFER_SIZE_MB);

    uint64_t addr = virt_to_phys((uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % 1000)));
    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    int rows[32] = {-1};
    int rows_length = 0;
    for (int x = 0; x < 32; x++) {
        uint64_t addr0 = phys_to_virt(addr ^ (addr & (1 << x)));
        uint64_t addr1 = phys_to_virt(addr | (1  << x));
        if (addr0 == 0 || addr1 == 0) continue;
        for (int sleep = 0; sleep < 10000; sleep++);
        uint64_t time = 0;
        for (int k = 0; k < SAMPLES; k++) {
            time += measure_bank_latency(addr0, addr1);
        }
        time = time / SAMPLES;
        rows[x] = time;
    }

    //Print the Histogram
    printf("Total Number of addresses accessesed: %d\n", 32);
  
    puts("-------------------------------------------------------");
    printf("Bit\t\tLatency(cycles) \n");
    puts("-------------------------------------------------------");

    for (int i=0; i<32 ;i++){
        printf("Bit %d   \t %15ld \n",
	    i, rows[i]);
    }    
}