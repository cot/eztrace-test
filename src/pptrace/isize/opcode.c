/*
 * opcode.c
 *
 * Determining the size to override (by computing the size of instructions) using the opcode library
 *
 *  Created on: 2 aožt 2011
 *      Author: dmartin
 */

#include <opcodes.h>
#include <tracing.h>
#include <bfd.h>
#include <dis-asm.h>

int dummy_print(void *stream, const char *fmt) { return 0; }

ssize_t opcode_get_overriden_size(bfd *abfd, pid_t child, uint64_t symbol, size_t trampoline_size)
{
	size_t max_size = trampoline_size * 2;
	if(max_size < 10) max_size = 10;

	code = (unsigned char*)malloc(max_size);
	trace_read(child, symbol, code, max_size);
	INIT_DISASSEMBLE_INFO(i, NULL, dummy_print);

	i.arch = bfd_get_arch(abfd);
	i.mach = bfd_get_mach(abfd);
    disassembler_ftype disassemble = disassembler( abfd );

    i.buffer = code;
    i.buffer_vma = (unsigned long)i.buffer;
    i.buffer_length = max_size;

    ssize_t j;
	for(j = 0; j < trampoline_size;) {
		j += disassemble(i.buffer_vma + j, &i);
	}
	free(code);

	return j;
}

ssize_t get_overriden_size(void* bin, pid_t child, uint64_t symbol, size_t trampoline_size)
{
	return opcode_get_overriden_size((bfd*)bin, child, symbol, trampoline_size);
}
