#include "shared.hh"
#include "params.hh"
#include "util.hh"
#include <errno.h>
#include <emmintrin.h>

// Physical Page Number to Virtual Page Number Map
std::map<uint64_t, uint64_t> PPN_VPN_map;

// Base pointer to a large memory pool
void * allocated_mem;

uint64_t get_frame_number(uint64_t addr) {
    return addr >> PAGE_SIZE_BITS;
}

uint64_t get_offset(uint64_t addr) {
    return addr & ((1ULL << PAGE_SIZE_BITS) - 1);
}

/*
 * setup_PPN_VPN_map
 *
 * Populates the Physical Page Number -> Virtual Page Number mapping table
 *
 * Inputs: mem_map - Base pointer to the large allocated pool
 *         PPN_VPN_map - Reference to a PPN->VPN map 
 *
 * Side-Effects: For *each page* in the allocated pool, the virtual page 
 *               number is into the map with a key corresponding to the 
 *               page's physical page number.
 *
 */
void setup_PPN_VPN_map(void * mem_map, uint64_t memory_size) {
    for (uint64_t i = 0; i < memory_size; i += PAGE_SIZE) {
        uint64_t * addr = (uint64_t *) ((uint8_t *) (mem_map) + i);
        uint64_t vpn = get_frame_number((uint64_t)addr);
        uint64_t ppn = get_frame_number(virt_to_phys((uint64_t)addr));
        PPN_VPN_map[ppn] = vpn;
    } 
}

/*
 * allocate_pages
 *
 * Allocates a memory block of size BUFFER_SIZE_MB
 *
 * Make sure to write something to each page in the block to ensure
 * that the memory has actually been allocated!
 *
 * Inputs: none
 * Outputs: A pointer to the beginning of the allocated memory block
 */
void * allocate_pages(uint64_t memory_size) {
    void * memory_block = mmap(NULL, memory_size, PROT_READ | PROT_WRITE,
        MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE , -1, 0);
  
    if (memory_block == (void*) - 1) {
        printf("memory allocation failed: %s\n", strerror(errno));
    }

    for (uint64_t i = 0; i < memory_size; i += PAGE_SIZE) {
        uint64_t * addr = (uint64_t *) ((uint8_t *) (memory_block) + i);
        *addr = i;
    } 

    return memory_block;  
}

void deallocate_pages(void *memory_block, uint64_t memory_size) {
    if (memory_block == NULL || memory_size == 0) {
        return;
    }
    munmap(memory_block, memory_size);
}

/* 
 * virt_to_phys
 *
 * Determines the physical address mapped to by a given virtual address
 *
 * Inputs: virt_addr - A virtual pointer/address
 * Output: phys_ptr - The physical address corresponding to the virtual pointer
 *                    IMPORTANT: If the virtual pointer is not currently present, return 0
 *
 */

uint64_t virt_to_phys(uint64_t virt_addr) {
    uint64_t phys_addr = 0;

    FILE * pagemap;
    uint64_t entry;

    // Compute the virtual page number from the virtual address
    uint64_t virt_page_offset = get_offset(virt_addr);
    uint64_t virt_page_number = get_frame_number(virt_addr);
    uint64_t file_offset = virt_page_number * sizeof(uint64_t);

    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), file_offset, SEEK_SET) == file_offset) {
            if (fread(&entry, sizeof(uint64_t), 1, pagemap)) {
                // 1ULL = 1 unsigned long long
                if (entry & (1ULL << 63)) { 
                    uint64_t phys_page_number = entry & ((1ULL << 54) - 1);
                    phys_addr = (phys_page_number << PAGE_SIZE_BITS) | virt_page_offset;
                } 
            }
        }
        fclose(pagemap);
    }

    return phys_addr;
}

/*
 * phys_to_virt
 *
 * Determines the virtual address mapping to a given physical address
 *
 * HINT: This should use your PPN_VPN_map!
 *
 * Inputs: phys_addr - A physical pointer/address
 * Output: virt_addr - The virtual address corresponding to the physical pointer
 *                     If the physical pointer is not mapped, return 0
 *
 */

uint64_t phys_to_virt(uint64_t phys_addr) {
    uint64_t virt_addr = 0;
    uint64_t ppn = get_frame_number(phys_addr);
    uint64_t offset = get_offset(phys_addr);
    uint64_t vpn = PPN_VPN_map[ppn];
    if (vpn != 0) {
        virt_addr = (vpn << PAGE_SIZE_BITS) | offset;
    }
    return virt_addr;
}

/*
 * measure_bank_latency
 *
 * Measures a (potential) bank collision between two addresses,
 * and returns its timing characteristics.
 *
 * Inputs: addr_A/addr_B - Two (virtual) addresses used to observe
 *                         potential contention
 * Output: Timing difference (derived by a scheme of your choice)
 *
 */
uint64_t measure_bank_latency2(uint64_t addr_A, uint64_t addr_B) {
    clflush(addr_A);
    clflush(addr_B);

    // make sure row buffer has A
    maccess(addr_A);  
    clflush(addr_A);
  
    return two_maccess_t(addr_A, addr_B);  
}

uint64_t measure_bank_latency3(uint64_t addr_X, uint64_t addr_A, uint64_t addr_B) {
    clflush(addr_X);
    clflush(addr_A);
    clflush(addr_B);

    // make sure row buffer has X
    maccess(addr_X);  
    clflush(addr_X);
  
    return three_maccess_t(addr_X, addr_A, addr_B);  
}

char *int_to_binary(uint64_t num, int num_bits) {
    char *binary = (char *) calloc(num_bits + 1, 1);
	
    for (int i = num_bits - 1; i >= 0; i--) {
        binary[i] = (num & 1) + '0';
        num >>= 1;
    }
    binary[num_bits] = '\0';
    return binary;
}

uint64_t two_address_access_latency(uint64_t a, uint64_t b) {
    uint64_t two_address_access = 0;
    for (int trials = 0; trials < SAMPLES; trials++){
        two_address_access += measure_bank_latency2(a, b);
    }
    two_address_access = two_address_access / SAMPLES;
    return two_address_access;
}

uint64_t three_address_access_latency(uint64_t x, uint64_t a, uint64_t b) {
    uint64_t three_address_access = 0;
    for (int trials = 0; trials < SAMPLES; trials++){
        three_address_access += measure_bank_latency3(x, a, b);
    }
    three_address_access = three_address_access / SAMPLES;
    return three_address_access;
}

void flush_row(uint8_t *row) {
    // 64 byte cache line
    for (uint32_t index = 0; index < ROW_SIZE / sizeof(uint8_t); index++) {
        clflush((uint64_t)(row + index));
    }
}

uint64_t get_dram_address(uint64_t row, int bank, uint64_t col) {
    int bank_xor_bits = (row & 0x7) ^ bank;
    return (row << 16) | (bank_xor_bits << 13) | col;
}

bool get_addresses_to_hammer(uint64_t victim, uint64_t *attacker_1, uint64_t *attacker_2, int row_diff) {
    int tries = 1000;
    while (tries-- > 0) {
        uint64_t phys_addr = virt_to_phys(victim);
        uint64_t row = phys_addr >> 16;
        uint64_t col = phys_addr & 0x1fff; // 13 bits 
        int bank_xor_bits = (phys_addr >> 13) & 0x7;
        int bank = bank_xor_bits ^ (row & 0x7);

        *attacker_1 = phys_to_virt(get_dram_address(row + row_diff, bank, col));
        *attacker_2 = phys_to_virt(get_dram_address(row - row_diff, bank, col));
        if (*attacker_1 != 0 && *attacker_2 != 0) return true;
    }
    return false;
}

uint32_t count_flips(uint8_t* victim, uint8_t expected) {
    uint32_t number_of_bitflips = 0;
    for (uint32_t index = 0; index < ROW_SIZE; index++) {
        if (victim[index] != expected) {
            number_of_bitflips++;
        }
    }
    return number_of_bitflips;
}

void print_diff(uint8_t* victim, uint8_t expected) {
    for (uint32_t index = 0; index < ROW_SIZE; index++) {
        if (victim[index] != expected) {
            printf("Located bit flip in byte %d\n", index);
            printf(RED "%s\n" RESET, int_to_binary(victim[index], 8));
            printf("%s\n\n", int_to_binary(expected, 8));
        }
    }
}

void print_result(uint64_t victim, uint64_t attacker_1, uint64_t attacker_2, uint32_t num_bit_flips) {
    uint64_t x = virt_to_phys(victim);
    uint64_t a = virt_to_phys(attacker_1);
    uint64_t b = virt_to_phys(attacker_2);
    printf("victim: %s\t%ld (phys)\n", int_to_binary(x, 33), x);
    printf("attacker 1: %s\t%ld (phys)\n", int_to_binary(a, 33), a);
    printf("attacker 2: %s\t%ld (phys)\n", int_to_binary(b, 33), b);
    printf("Bit flips found: %d\n", num_bit_flips);
}