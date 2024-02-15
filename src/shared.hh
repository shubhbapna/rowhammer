#ifndef SHARED_GUARD
#define SHARED_GUARD

#include <fcntl.h>
#include <inttypes.h>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <string.h>

#include <unistd.h>
#include <vector>
#include <cstdint>
#include <emmintrin.h>


// Physical Page Number to Virtual Page Number Map
extern std::map<uint64_t, uint64_t> PPN_VPN_map;

// Base pointer to a large memory pool
extern void * allocated_mem;

uint64_t virt_to_phys(uint64_t virt_addr);
uint64_t phys_to_virt(uint64_t phys_addr);
void setup_PPN_VPN_map(void * mem_map, uint64_t memory_size);
uint64_t measure_bank_latency2(uint64_t addr_A, uint64_t addr_B);
uint64_t measure_bank_latency3(uint64_t addr_X, uint64_t addr_A, uint64_t addr_B);
void * allocate_pages(uint64_t memory_size);
char *int_to_binary(uint64_t num, int num_bits);
uint64_t two_address_access_latency(uint64_t a, uint64_t b);
uint64_t three_address_access_latency(uint64_t x, uint64_t a, uint64_t b);
void flush_row(uint8_t *row);
uint64_t get_dram_address(uint64_t row, int bank, uint64_t col);
bool get_addresses_to_hammer(uint64_t victim_phys_addr, uint64_t *attacker_1, uint64_t *attacker_2, int row_diff);
void print_result(uint64_t victim, uint64_t attacker_1, uint64_t attacker_2, uint32_t num_bit_flips);
uint32_t count_flips(uint8_t* victim, uint8_t expected);
void print_diff(uint8_t* victim, uint8_t expected);
void deallocate_pages(void *memory_block, uint64_t memory_size);

#endif
