#ifndef PARAMS_GUARD
#define PARAMS_GUARD

#define RED   "\x1B[31m"
#define RESET "\x1B[0m"

// Size of allocated buffer = 2GB
#define BUFFER_SIZE 2147483648UL

// Size of hugepages in system
#define PAGE_SIZE (1 << 12)

// Size of DRAM row (1 bank)
#define ROW_SIZE (8192)

// Number of hammers to perform per iteration: hammer 10x refresh period
#define HAMMERS_PER_ITER 4000000

// Number of times to repeat pressing so as to synchronize with refreshes
#define PRESS_PER_ITER 800000

// Number of cache blocks to access in the same row
#define PRESS_NUM_READS 32

// Number of times to press the aggressors
#define PRESS_NUM_AGGR_ACTS 2

// number of sample for measuring bank latency
#define SAMPLES 100

#define PAGE_SIZE_BITS 12

//Num Latency Buckets.
#define NUM_LAT_BUCKETS (50)

//Each Bucket Step.
#define BUCKET_LAT_STEP (20)

#define ROW_BUFFER_CONFLICT_LATENCY (390)

#define ROW_BUFFER_HIT_LATENCY (290)

#ifndef VERBOSE
    #define VERBOSE 0
#endif

#endif
