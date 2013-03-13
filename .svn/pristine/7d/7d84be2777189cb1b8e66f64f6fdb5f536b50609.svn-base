/*
 * opcodes.h -- Opcode generation
 *
 *  - insert_opcodes(opcode, opcode_size, buf, size, offset): insert *opcode*
 *  	of size *opcode_size* at offset *offset* of *buf* of size *size*.
 *  	Returns min(offset+opcode_size, size)
 *  - generate_trampoline(buf, size, orig_addr, reloc_addr): generates
 *  	opcodes to jump to *reloc_addr* from *orig_addr*. The opcodes are the
 *  	smallest possible.
 *  	The generated opcodes are put at the beginning of *buf* (that is
 *  	of size *size*). Returns the new size of the inserted opcodes.
 *  - read_jump(buf, size, offset, prefix): read a jump part that was prefixed
 *  	by *prefix* and that is contained at offset *offset* of *buf* (*size* is
 *  	the size of *buf*). The returned value is the size of the instruction
 *  	if a jump is read, 0 if a valid prefix for a jump was encountered, and < 0
 *  	if the instruction is not a jump.
 *
 *  - MAX_TRAMPOLINE_SIZE is a constant giving the maximum size of a trampoline
 *     overwrite
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#ifndef PPTRACE_OPCODES_H_
#define PPTRACE_OPCODES_H_
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_TRAMPOLINE_SIZE 48 // Be large :)

ssize_t insert_opcodes(uint8_t* opcode, int opcode_size, uint8_t* buf, size_t size, size_t offset);

ssize_t generate_trampoline(uint8_t* buffer, size_t size, uint64_t orig_addr, uint64_t reloc_addr);

ssize_t read_jump(uint8_t* buf, size_t size, size_t offset, uint8_t prefix);

#endif /* PPTRACE_OPCODES_H_ */
