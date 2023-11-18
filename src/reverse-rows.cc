#include "shared.hh"
#include "util.hh"
#include "params.hh"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    uint64_t addr = virt_to_phys((uint64_t)(allocated_mem + ROW_SIZE * 1000));
    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    int rows[32] = {-1};
    int rows_length = 0;
    for (int x = 0; x < 32; x++) {
        uint64_t addr0 = addr ^ (addr & (1 << x));
        uint64_t addr1 = addr | (1  << x);
        uint64_t time = 0;
        for (int k = 0; k < SAMPLES; k++) {
            time += measure_bank_latency(addr0, addr1);
        }
        time = time / SAMPLES;
        bank_lat_histogram[time / BUCKET_LAT_STEP]++;
    }

    //Print the Histogram
    printf("Total Number of addresses accessesed: %ld\n", 32);
  
    puts("-------------------------------------------------------");
    printf("Latency(cycles)\t\tAddress-Count \n");
    puts("-------------------------------------------------------");

    for (int i=0; i<NUM_LAT_BUCKETS ;i++){
        printf("[%d-%d)   \t %15ld \n",
	    i*BUCKET_LAT_STEP, i*BUCKET_LAT_STEP + BUCKET_LAT_STEP, bank_lat_histogram[i]);
    }
    printf("%d+   \t %15ld \n",NUM_LAT_BUCKETS*BUCKET_LAT_STEP, bank_lat_histogram[NUM_LAT_BUCKETS]);

    
}