/*
 * trace.c
 *
 * Determining the size to override (by computing the size of instructions) using single step tracing
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <opcodes.h>
#include <tracing.h>
#include <errors.h>
#include <sys/types.h>
#include <sys/wait.h>

ssize_t trace_get_jump_size(pid_t child, uint64_t instruction_addr)
{
	uint8_t buf[10];
	size_t offset = 0;
	uint8_t prefix = 0;
	trace_read(child, instruction_addr, buf, 10);
	ssize_t r = read_jump(buf, 10, offset, prefix);
	while (!r && offset < 9) {
		prefix = buf[offset];
		offset++;
		r = read_jump(buf, 10, offset, prefix);
	}

	if(r > 0) {
		return r + offset; // it's a jump of r + offset bytes
	}
	return 0;
}

ssize_t get_overriden_size(void *bin, pid_t child, uint64_t symbol, size_t trampoline_size)
{
	// Determine the size of instructions to replace
	// To do so, place rip on the symbol and singlestep until the size of instruction exceeed trampoline_size
	// Returns the size of the replacement

	REG_STRUCT regs;
	trace_get_regs(child, &regs); // Backup registers

	set_ip(child, symbol);        // Jump to symbol
	uint64_t crip = symbol;
	while (crip < symbol + trampoline_size) {
		int status = trace_singlestep(child); // single stepping until we go further than symbol + trampoline_size
		uint64_t newrip = get_ip(child);
		// Unfortunately, this works only if no jump / call
		if(newrip < crip || newrip > crip + 2) { // There might be a jump
			// Tells wether it's a jump or not
			ssize_t off = trace_get_jump_size(child, crip);
			if (off) {
				newrip = crip + off;
				set_ip(child, newrip);
			} else if(newrip < crip) {
				trace_set_regs(child, &regs); // Restore registers
				return 0; // We have failed!
			}
		}
		else if (newrip == crip) {
			if (WIFSTOPPED(status)) {
				newrip = crip; // Stopped, continue
			}
			else {
				pptrace_fubar("got signal %d (term) %d (status)", WTERMSIG(status), WSTOPSIG(status));
			}
		}
		crip = newrip;
	}
	trace_set_regs(child, &regs); // Restore registers
	return (size_t)(crip - symbol);
}
