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

/* Flush a cache block of address "addr" */
static inline void mfence()
{
  asm volatile ("mfence"
		: /*output*/
		: /*input*/
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
static inline uint64_t maccess_t(uint64_t addr)
{
	uint64_t cycles;

	asm volatile("mov %1, %%r8\n\t"
	"lfence\n\t"
	"rdtsc\n\t"
	"mov %%eax, %%edi\n\t"
	"mov (%%r8), %%r8\n\t"
	"lfence\n\t"
	"rdtsc\n\t"
	"sub %%edi, %%eax\n\t"
	: "=a"(cycles) /*output*/
	: "r"(addr)    /*input*/
	: "r8", "edi");	/*reserved register*/

	return cycles;
}

#endif // _UTILITY_H__ 

