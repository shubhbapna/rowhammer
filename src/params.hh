#ifndef PARAMS_GUARD
#define PARAMS_GUARD

// Size of allocated buffer = 2GB
#define BUFFER_SIZE_MB 2147483648UL

// Size of hugepages in system
#define PAGE_SIZE (1 << 12)

// Size of DRAM row (1 bank)
#define ROW_SIZE (8192)

// Number of hammers to perform per iteration
#define HAMMERS_PER_ITER 5000000

// number of sample for measuring bank latency
#define SAMPLES 1000

#define PAGE_SIZE_BITS 12

//Num Latency Buckets.
#define NUM_LAT_BUCKETS (20)

//Each Bucket Step.
#define BUCKET_LAT_STEP (50)

#endif
