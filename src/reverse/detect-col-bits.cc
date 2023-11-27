#include "../shared.hh"
#include "../util.hh"
#include "../params.hh"
#include <cstdlib>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    // 90% of phy mem
    uint64_t mem_size = 1.8 * BUFFER_SIZE_MB;
    allocated_mem = allocate_pages(mem_size);
    setup_PPN_VPN_map(allocated_mem, mem_size);
    
    uint64_t rows[32] = {0};
    int tries = 1000;
    for (int x = 0; x < 32; x++) {
        uint64_t addr, addr0, addr1;
        while (tries-- > 0) {
            addr = virt_to_phys((uint64_t)((uint8_t *)allocated_mem + ROW_SIZE * (rand() % (mem_size / PAGE_SIZE))));
            addr0 = phys_to_virt(addr ^ (addr & (1 << x)));
            addr1 = phys_to_virt(addr | (1  << x));
            if (addr0 != 0 && addr1 != 0) break;
        }

        if (tries <= 0) continue;
        rows[x] = two_address_access_latency(addr0, addr1);
    }

    for (int i = 0; i < 32; i++) {
        printf("Bit %d: %10ld", i, rows[i]);        
    }
}