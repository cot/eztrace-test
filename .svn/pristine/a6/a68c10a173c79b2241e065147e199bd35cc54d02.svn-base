/*
 * isize.h -- determining instruction size
 *
 * Provided method:
 * 	- get_overriden_size(bin, child, symbol, trampoline_size): determines
 * 		how much of *symbol* should be copied to replay the part overriden by
 * 		a trampoline of size *trampoline_size*. *bin* is the opaque structure
 * 		of the binary and *child* the pid of the child process.
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#ifndef PPTRACE_ISIZE_H_
#define PPTRACE_ISIZE_H_
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

ssize_t get_overriden_size(void* bin, pid_t child, uint64_t symbol, size_t trampoline_size);

#endif /* PPTRACE_ISIZE_H_ */
