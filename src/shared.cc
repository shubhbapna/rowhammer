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
uint64_t measure_bank_latency(uint64_t addr_A, uint64_t addr_B) {
  clflush(addr_A);
  clflush(addr_B);

  maccess(addr_A);
  maccess(addr_B);
  
  clflush(addr_A);
  
  return maccess_t(addr_A);  
}
