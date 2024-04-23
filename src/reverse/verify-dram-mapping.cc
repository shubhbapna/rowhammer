#include "../shared.hh"
#include "../util.hh"
#include "../params.hh"
#include <set>
#include <cstdlib>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Hypothesis:\n");
    printf("\tCol bits: 0-12\n");
    printf("\tBank xor bits: 13-15\n");
    printf("\tRank bits: 16\n");
    printf("\tRow bits: 17-31\n");
    printf("Confirming...\n");

    uint64_t mem_size = 1.8 * BUFFER_SIZE;
    allocated_mem = allocate_pages(mem_size);
    setup_PPN_VPN_map(allocated_mem, mem_size);

    uint64_t addr, addr_a, addr_b;
    int tries = 1000;
    while (tries-- > 0) {
        addr = (uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % (mem_size / PAGE_SIZE)));
        uint64_t phys_addr = virt_to_phys(addr);
        uint64_t row = phys_addr >> 17;
	uint64_t rank = phys_addr & 1 << 16;
        uint64_t col = phys_addr & 0x1fff; // 13 bits 
        int bank_xor_bits = (phys_addr >> 13) & 0x7;
        int bank = bank_xor_bits ^ (row & 0x7);

        addr_a = phys_to_virt(get_dram_address(row + 1, bank, col, rank));
        addr_b = phys_to_virt(get_dram_address(row - 1, bank, col, rank));
        if (addr_a != 0 && addr_b != 0) break;
    }

    if (tries <= 0) {
        printf("Could not get test address. Try again\n");
        exit(1);
    }

    uint64_t two_address_access_time = two_address_access_latency(addr, addr_a);
    uint64_t three_address_access_time = three_address_access_latency(addr, addr_a, addr_b);
    
    uint64_t x = virt_to_phys(addr);
    uint64_t a = virt_to_phys(addr_a);
    uint64_t b = virt_to_phys(addr_b);
    printf("X: %s\t%ld (phys)\n", int_to_binary(x, 33), x);
    printf("A: %s\t%ld (phys)\n", int_to_binary(a, 33), a);
    printf("B: %s\t%ld (phys)\n", int_to_binary(b, 33), b);
    printf("Access time for X and A: %ld\n", two_address_access_time);
    printf("Access time for X, A, and B: %ld\n", three_address_access_time);
    if (
        two_address_access_time < 600 
        && two_address_access_time >= ROW_BUFFER_CONFLICT_LATENCY 
        && three_address_access_time > two_address_access_time + 120
    ) {
        printf("Hypothesis confirmed! Found row conflict\n");
    } else {
        printf("Try again\n");

    }
}
