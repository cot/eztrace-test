#include <opcodes.h>
#include <tracing.h>
#include <isize.h>
#include <memory.h>
#include <binary.h>
#include <errors.h>
#include <stdlib.h>

ssize_t hijack_code(void* bin, pid_t child, uint64_t sym_addr, uint64_t sym_size, uint64_t reloc_addr, uint64_t orig_addr, uint64_t repl_addr)
{
#define HIJACK_CODE_FAIL(test) if(test) { \
		free(trampoline); \
		free(trampoline2); \
		free(trampoline3); \
		return -1; \
	}

	 // If the size is not available, then we suppose we have enough room
	if(sym_size <= 0) {
		sym_size = MAX_TRAMPOLINE_SIZE;
	}
	uint8_t* trampoline = (uint8_t*)malloc(MAX_TRAMPOLINE_SIZE);
	uint8_t* trampoline2 = (uint8_t*)malloc(MAX_TRAMPOLINE_SIZE);
	uint8_t* trampoline3 = (uint8_t*)malloc(sym_size);

	// Building an intermediate trampoline for space reason (a long jump in the reloc space so that the trampoline is a short jump)
	ssize_t trampoline3_size = generate_trampoline(trampoline3, MAX_TRAMPOLINE_SIZE, reloc_addr, repl_addr);
	HIJACK_CODE_FAIL(trampoline3_size < 0);

	// Building the trampoline. This can fail if we don't have enough space in the hijacked symbol
	ssize_t trampoline_size = generate_trampoline(trampoline, sym_size, sym_addr, reloc_addr);
	HIJACK_CODE_FAIL(trampoline_size < 0);

	// And now ptrace insertion...

	// Copy the original function into the dest process. BUG? We should make sure all calls and jump stays correct... (how?)
	ssize_t over = get_overriden_size(bin, child, sym_addr, trampoline_size); // get the size to override
	HIJACK_CODE_FAIL(over < trampoline_size || over > sym_size);

	ssize_t trampoline2_size = generate_trampoline(trampoline2, sym_size, reloc_addr+over+trampoline3_size, sym_addr+over); // create the trampoline back to the original address
	HIJACK_CODE_FAIL(trampoline2_size < 0);

	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "(hijack)\tinserting trampoline from relocated code (0x%lx) to interception function (0x%lx)\n", (unsigned long)reloc_addr, (unsigned long)repl_addr);
	trace_write(child, reloc_addr, trampoline3, trampoline3_size); // Insert the long jump
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "(hijack)\tcopying overwritten code (%d bytes) from original code (0x%lx) to relocated position (0x%lx)\n", over, (unsigned long)sym_addr, (unsigned long)(reloc_addr+trampoline3_size));
	trace_copy(child, sym_addr, reloc_addr+trampoline3_size, over); // insert the relocated code
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "(hijack)\tinserting trampoline from relocated code (0x%lx) to original position (0x%lx)\n", (unsigned long)(reloc_addr+trampoline3_size+over), (unsigned long)(sym_addr+over));
	trace_write(child, reloc_addr+trampoline3_size+over, trampoline2, trampoline2_size); // insert the trampoline back to the original function

	// Insert the trampoline into the dest process
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "(hijack)\tinserting trampoline from original position (0x%lx) to relocated code (0x%lx)\n", (unsigned long)sym_addr, (unsigned long)reloc_addr);
	trace_write(child, sym_addr, trampoline, trampoline_size);
	free(trampoline);
	free(trampoline2);
	free(trampoline3);

	// Now only the orig_addr needs to be written with the reloc_addr
	reloc_addr += trampoline3_size;
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "(hijack)\tcopying relocated code addr (0x%lx) to original symbol (0x%lx)\n", (unsigned long)reloc_addr, (unsigned long)orig_addr);
	trace_write(child, orig_addr, (uint8_t*)(&reloc_addr), sizeof(reloc_addr));

	// Return the size of the inserted code
	return over+trampoline3_size+trampoline2_size;
}

ssize_t hijack(void* bin, pid_t child, zzt_symbol *toHijack, zzt_symbol *orig, zzt_symbol *repl)
{
	// Compute the addresses from the symbols
	uint64_t addr = (uint64_t)(toHijack->symbol_offset+toHijack->section_addr);
	uint64_t sym_size = (uint64_t)(toHijack->symbol_size);
	uint64_t orig_addr = (uint64_t)(orig->symbol_offset + orig->section_addr);
	uint64_t repl_addr = (uint64_t)(repl->symbol_offset + repl->section_addr);

	pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Hijacking symbol %s (0x%lx) with %s (0x%lx) using %s (0x%lx) as function pointer...\n",
				toHijack->symbol_name, (unsigned long)addr, repl->symbol_name, (unsigned long)repl_addr, orig->symbol_name, (unsigned long)orig_addr);
	pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Allocating buffer for relocating bytes... ");
	uint64_t reloc_addr = allocate_buffer(child, MAX_TRAMPOLINE_SIZE);
	if(reloc_addr != 0) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "ok (0x%lx)\n", (unsigned long)reloc_addr);
		ssize_t hijack_size = hijack_code(bin, child, addr, sym_size, reloc_addr, orig_addr, repl_addr);
		if (hijack_size > 0 && hijack_size != MAX_TRAMPOLINE_SIZE) {
			correct_buffer_allocation(child, MAX_TRAMPOLINE_SIZE, hijack_size);
		}
		pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Symbol hijacked...\n");
		return hijack_size;
	}
	pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "failed!\n");
	return -1;
}
