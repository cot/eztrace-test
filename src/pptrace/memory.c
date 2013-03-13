/*
 * memory.c
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <memory.h>
#include <tracing.h>
#include <sys/mman.h>

// Memory allocation. Don't handle memory free (we don't care actually)
#define MY_PAGE_SIZE (1024*1024) // We use 1Mbytes pages for allocation
uint64_t currentPointer    = 0; // The current pointer toward free memory
uint64_t currentEndPointer = 0; // The pointer to the end of the current page
uint64_t allocate_buffer(pid_t child, size_t size)
{
	// This algorithm is dummy, because we don't need to be memory efficient for the kind of target we plan
	if(currentPointer-currentEndPointer < size) {
		// We need a new page

		uint64_t pagesize = MY_PAGE_SIZE;
		while (pagesize < size) pagesize += MY_PAGE_SIZE;

		currentPointer = trace_mmap(child, 0, pagesize, PROT_READ | PROT_EXEC);

		if (currentPointer) {
			currentEndPointer = currentPointer+pagesize;
		}
		else {
			currentEndPointer = 0;
			return 0;
		}
	}
	// Do the dummy allocation
	uint64_t r = currentPointer;
	currentPointer += size;
	return r;
}


void correct_buffer_allocation(pid_t child, size_t required_size, size_t actual_size)
{
	currentPointer -= (required_size-actual_size);
}

