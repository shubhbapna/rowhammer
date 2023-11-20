#include "../shared.hh"
#include "../util.hh"
#include "../params.hh"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    const long int num_iterations = BUFFER_SIZE_MB / ROW_SIZE;
    char *base = (char *)allocated_mem;
    for (int i = 1; i < num_iterations; i++) {
        uint64_t time = 0;
        for (int j = 0; j < SAMPLES; j++) {
            time += measure_bank_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        }
        time = time / SAMPLES;
        bank_lat_histogram[time / BUCKET_LAT_STEP]++;
    }

    //Print the Histogram
    printf("Total Number of addresses accessesed: %ld\n", num_iterations);
  
    puts("-------------------------------------------------------");
    printf("Latency(cycles)\t\tAddress-Count \n");
    puts("-------------------------------------------------------");

    for (int i=0; i<NUM_LAT_BUCKETS ;i++){
        printf("[%d-%d)   \t %15ld \n",
	    i*BUCKET_LAT_STEP, i*BUCKET_LAT_STEP + BUCKET_LAT_STEP, bank_lat_histogram[i]);
    }
    printf("%d+   \t %15ld \n",NUM_LAT_BUCKETS*BUCKET_LAT_STEP, bank_lat_histogram[NUM_LAT_BUCKETS]);
}