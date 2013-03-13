/*
 * memory.h -- handles memory allocation in the target process
 *
 * Provided methods:
 * 	- allocate_buffer(child, size): allocate a buffer of size *size* in
 * 		the process *child*. Returns the address allocated
 *	- correct_buffer_allocation(child, required_size, actual_size): free
 *		(*required_size*-*actual_size*) in the process *child* for the last
 *		allocation made by allocate_buffer().
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#ifndef PPTRACE_MEMORY_H_
#define PPTRACE_MEMORY_H_
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t allocate_buffer(pid_t child, size_t size);
void correct_buffer_allocation(pid_t child, size_t required_size, size_t actual_size);

#endif /* PPTRACE_MEMORY_H_ */
