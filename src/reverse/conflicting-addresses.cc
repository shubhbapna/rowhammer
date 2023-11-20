#include "../shared.hh"
#include "../util.hh"
#include "../params.hh"
#include <set>
#include <cstdlib>

char *int_to_binary(uint64_t num)
{
    int num_bits = 33;

    char *binary = (char *) calloc(num_bits + 1, 1);
	
    for (int i = num_bits - 1; i >= 0; i--) {
        binary[i] = (num & 1) + '0';
        num >>= 1;
    }
    binary[num_bits] = '\0';
    return binary;
}

uint64_t two_address_latency(uint64_t a, uint64_t b) {
    uint64_t two_address_access = 0;
    for (int trials = 0; trials < SAMPLES; trials++){
        two_address_access += measure_bank_latency(a, b);
    }
    two_address_access = two_address_access / SAMPLES;
    return two_address_access;
}

uint64_t three_address_latency(uint64_t x, uint64_t a, uint64_t b) {
    uint64_t three_address_access = 0;
    for (int trials = 0; trials < SAMPLES; trials++){
        three_address_access += measure_bank_latency3(x, a, b);
    }
    three_address_access = three_address_access / SAMPLES;
    return three_address_access;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    allocated_mem = allocate_pages(BUFFER_SIZE_MB);
    uint64_t* bank_lat_histogram = (uint64_t*) calloc((NUM_LAT_BUCKETS+1), sizeof(uint64_t));
    
    const long int num_iterations = BUFFER_SIZE_MB / ROW_SIZE;
    char *base = (char *)allocated_mem;
    std::set<int> conflicts;

    for (int i = 1; i < num_iterations; i++) {
        uint64_t time = two_address_latency((uint64_t)base, (uint64_t)(base + i * ROW_SIZE));
        // keeping less than 500 to avoid noise
        if (ROW_BUFFER_CONFLICT_LATENCY <= time && time < 600) {
            conflicts.insert(i);
        }
    }
    int flag = 0;
    int index_a, index_b;
    for (int a: conflicts) {
        uint64_t two_address_access = two_address_latency((uint64_t)base, (uint64_t)(base + a * ROW_SIZE));
        
        for (int b: conflicts) {
            if (a == b) break;
            uint64_t three_address_access = three_address_latency((uint64_t)base, (uint64_t)(base + a * ROW_SIZE), (uint64_t)(base + b * ROW_SIZE));
            if (three_address_access > two_address_access + 120) {
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
    printf("X: %s\t%ld (phys)\t%ld (virt)\n", int_to_binary(x), x, (uint64_t) base);
    printf("A: %s\t%ld (phys)\t%d (virt)\n", int_to_binary(a), a, index_a);
    printf("B: %s\t%ld (phys)\t%d (virt)\n", int_to_binary(b), b, index_b);

    printf("\nHypothesis:\n");
    printf("\tCol bits: 0-12\n");
    printf("\tBank xor bits: 13-15\n");
    printf("\tRow bits: 16-31\n");
    printf("Confirming...\n");

    uint64_t addr, addr_a, addr_b;
    int tries = 1000;
    while (tries-- > 0) {
        addr = (uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % num_iterations));
        uint64_t virt_addr = virt_to_phys(virt_addr);
        uint64_t row = (virt_addr >> 16) & 0xffff;
        uint64_t remaining_bits = virt_addr & 0xffff;

        addr_a = phys_to_virt(((row + 1) << 16) | remaining_bits); // addr + 1
        addr_b = phys_to_virt(((row - 1) << 16) | remaining_bits); // addr - 1
    }

    if (tries <= 0) {
        printf("Could not verify try again\n");
    }

    uint64_t two_address_access = two_address_latency(addr, addr_a);
    uint64_t three_address_access = three_address_latency(addr, addr_a, addr_b);
    if (two_address_access < 600 && two_address_access >= ROW_BUFFER_CONFLICT_LATENCY && three_address_access > two_address_access + 120) {
        printf("Hypothesis confirmed! Found row conflict\n");
    } else {
        printf("Try again\n");
    }
}