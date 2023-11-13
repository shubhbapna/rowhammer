#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#ifndef __UTILITY_H__
#define __UTILITY_H__

/* Flush a cache block of address "addr" */
static inline void clflush(uint64_t addr)
{
  asm volatile ("clflush (%0)"
		: /*output*/
		: /*input*/ "r"(addr)
		: /*clobbers*/ );
}


/* Load address "addr" */
static inline void maccess(uint64_t addr)
{
  asm volatile("mov (%0), %%rax"
	       : /*output*/
	       : /*input*/ "r"(addr)
	       : /*clobbers*/ "rax" );
  
  return;
}


/* Loads addr and measure the access time */
static inline uint64_t maccess_t(uint64_t addr)
{
  uint64_t cycles;
  asm volatile(
        "mov %1, %%r8\n\t"
        "rdtscp\n\t" // stores higher 32 bits in edx and lower 32 in eax
        "shl $32, %%rdx\n\t"
        "or %%rdx, %%rax\n\t" // can add entire rax to rdx since rdtscp clears the higher 32 bits of rax
        "mov %%rax, %%rbx\n\t" // save start time in rbx
        "mov (%%r8), %%rax\n\t"
        "rdtscp\n\t"
        "shl $32, %%rdx\n\t"
        "or %%rdx, %%rax\n\t" // end time is store in rdx
        "sub %%rbx, %%rax" // subtract start time from end time and store in rax (cycles)
	       : /*output*/ "=a"(cycles) // "a" stands for rax/eax registr
	       : /*input*/  "r"(addr) // "r" is any register
         : "rdx", "rbx");    
  return cycles;
}

// Function to read the time stamp counter, which is called tsc for short
// "rdtscp" returns a 32bit unsigned integer
// "rdtscp64" return a 64 bit unsigned integer
// Details in https://www.felixcloutier.com/x86/rdtscp
// static inline uint32_t rdtscp() {
//   uint32_t rv;
//   asm volatile ("rdtscp": "=a" (rv) :: "edx", "ecx");
//   return rv;
// }

// static inline uint64_t rdtscp64() {
//   uint32_t low, high;
//   asm volatile ("rdtscp": "=a" (low), "=d" (high) :: "ecx");
//   return (((uint64_t)high) << 32) | low;
// }

// Function "lfence" wrap the assembly instruction lfence
// This function performs a serializing operation which ensures that
// the instructions after "lfence" start execution after
// all the instructions before "lfence" complete
// Details in https://www.felixcloutier.com/x86/lfence
// static inline void lfence() {
//   asm volatile("lfence");
// }

// Here is an example of using "rdtsc" and "lfence" to
// measure the time it takes to access a block specified by its virtual address
// The corresponding pseudo code is
// =========
// t1 = rdtsc
// load addr
// t2 = rdtsc
// cycles = t2 - t1
// return cycles
// =========
// static inline uint64_t measure_one_block_access_time(uint64_t addr)
// {
// 	uint64_t cycles;

// 	asm volatile("mov %1, %%r8\n\t"
// 	"lfence\n\t"
// 	"rdtsc\n\t"
// 	"mov %%eax, %%edi\n\t"
// 	"mov (%%r8), %%r8\n\t"
// 	"lfence\n\t"
// 	"rdtsc\n\t"
// 	"sub %%edi, %%eax\n\t"
// 	: "=a"(cycles) /*output*/
// 	: "r"(addr)    /*input*/
// 	: "r8", "edi");	/*reserved register*/

// 	return cycles;
// }


// static inline void one_block_access(uint64_t addr)
// {
// 	asm volatile("mov (%0), %%r8\n\t"
// 	: /*output*/
// 	: "r"(addr)    /*input*/
// 	: "r8");	/*reserved register*/

// }


// A wrapper function of the clflush instruction
// The instruction evict the given address from the cache to DRAM
// so that the next time the line is accessed, it will be fetched from DRAM
// Details in https://www.felixcloutier.com/x86/clflush
// static inline void clflush(uint64_t v) {
//   asm volatile ("clflush (%0)": : "r" (v):);
// }

// inline uint64_t get_time() {
// 	uint64_t cycles;
// 	asm volatile("rdtscp\n\t"
// 				 "shl $32, %%rdx\n\t"
// 				 "or %%rdx, %0\n\t"
// 				 : "=a"(cycles)
// 				 :
// 				 : "rcx", "rdx", "memory");

// 	return cycles;
// }

#endif // _UTILITY_H__ 

