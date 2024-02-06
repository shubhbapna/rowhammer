#include "../shared.hh"
#include "../params.hh"
#include "../util.hh"
#include <unistd.h>
#include <set>

uint32_t press(uint64_t vict_virt_addr, uint64_t attacker_virt_addr_1, uint64_t attacker_virt_addr_2) {
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
  
    for (int iter = 0; iter < PRESS_PER_ITER; iter++) {
        for (int i = 0; i < PRESS_NUM_AGGR_ACTS; i++) {
            for (int j = 0; j < PRESS_NUM_READS; j++) {
                maccess((uint64_t)(attacker_virt_addr_1_ptr + j));
            }

            for (int j = 0; j < PRESS_NUM_READS; j++) {
                maccess((uint64_t)(attacker_virt_addr_2_ptr + j));
            }

            for (int j = 0; j < PRESS_NUM_READS; j++) {
                clflush((uint64_t)(attacker_virt_addr_1_ptr + j));
                clflush((uint64_t)(attacker_virt_addr_2_ptr + j));
            }
            mfence();
        }
    }

    flush_row(vict_virt_addr_ptr);

    uint32_t number_of_bitflips_in_target = 0;
    for (uint32_t index = 0; index < ROW_SIZE; index++) {
        if (vict_virt_addr_ptr[index] != 0x55) {
            number_of_bitflips_in_target++;
        }
    }
    return number_of_bitflips_in_target; 
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
            uint32_t num_bit_flips = press(victim, *attacker_1, *attacker_2);
            print_result(victim, *attacker_1, *attacker_2, num_bit_flips);
            if (num_bit_flips > 0) break;
        }
    }
}

