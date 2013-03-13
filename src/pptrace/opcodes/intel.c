/*
 * intel.c
 *
 * Production of opcodes for 64-bit intel processors (aka. amd64 architecture).
 * This file should also be used for 32-bit targets (the program should adapt itself to the target).
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <stdlib.h>
#include <string.h>

ssize_t insert_opcodes(uint8_t* opcode, int opcode_size, uint8_t* buf, size_t size, size_t offset)
{
	int i = 0;
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "(opcodes)\tinserting opcodes: ");
	while (offset < size && i < opcode_size) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "%02X ", opcode[i]);
		buf[offset] = opcode[i];
		offset++; i++;
	}
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "\n");
	return offset;
}

ssize_t read_jump(uint8_t* buf, size_t size, size_t offset, uint8_t prefix)
{
	ssize_t offlen = (prefix == 0x66) ? 2 : 4; // if prefix is an instruction size override then it's 16-bits offset

	switch(buf[offset]) {
	case 0xEB: // JMP cb
	case 0xE3: // JrCXZ cb
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: // Jcc cb
	case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
		return 2;
	case 0x0f: // Possible Jcc cw/cd (0x0f 0x8x cd)
		if(size < offset + 2) return -2; // We need more byte to answer
		return  ((buf[offset + 1] & 0xf0) == 0x80) ? 2 + offlen : 0;

	case 0xE9: // JMP cw/cd
	case 0xE8: // CALL cw/cd
	case 0x9A: // CALL FAR cw/cd
		return 1 + offlen;

	case 0xFF: // JMP /4, JMP FAR /5, CALL /2, CALL FAR /3
		// Parse the ModRM byte
		if(size < offset + 2) return -2; // We need more byte to answer
		uint8_t modRM = buf[offset + 1];
		uint8_t mod = (modRM >> 6) & 0x3;
		uint8_t reg = (modRM >> 3) & 0x7;
		uint8_t rm  = (modRM) & 0x7;

		if (reg < 2 || reg > 5)
			return -1; // Neither CALL nor JMP

		switch(mod) {
		case 0: // Registers ( + SIB byte if rm = 4)
			return (rm == 4) ? 4 : 3;

		case 3: // Extended registers
			return 3;

		case 1: // offset16 ( + SIB byte if rm = 4)
			return (rm == 4) ? 5 : 4;

		case 2: // offset32 ( + SIB byte if rm = 4)
			return (rm == 4) ? 7 : 6;
		}
		return -1; // ???

		case 0xC3: // RET
		case 0xCB: // RET FAR
			return 1;

		case 0xC2: // RET cw
		case 0xCA: // RET FAR cw
			return 3;

			// Prefixes

		case 0x66: // Operand-Size Override
		case 0x67: // Address-Size Override
		case 0x2E: case 0x3E: case 0x26: case 0x64: case 0x65: case 0x36: // Segment Override
		case 0xF0: // Lock
		case 0xF3: case 0xF2: // Repeat
			return 0; // Retry with next byte

		default:
			return -1; // It's not a jump, nor a prefix
	}
}

inline int get_unsigned_nb_bits(uint64_t value)
{
	int index = 0;
	while(value) {
		value >>= 1;
		index++;
	}
	return index;
}

int get_signed_nb_bits(int64_t value)
{
	if(value < 0) {
		value = -value;
	} else {
		value++;
	}
	return get_unsigned_nb_bits(value)+1;
}

ssize_t generate_trampoline(uint8_t* buffer, size_t size, uint64_t orig_addr, uint64_t reloc_addr)
{
	uint8_t buf[12];
	int64_t offset = reloc_addr - orig_addr;
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "(opcodes)\tgenerating trampoline from 0x%lx to 0x%lx (delta is %ld)\n", (unsigned long)orig_addr, (unsigned long)reloc_addr, (unsigned long)offset);
	if (get_signed_nb_bits(offset - 2) <= 8) {
		if(size < 2) return -1;
		// 8-bit offset (jmp rel8off EB)
		buf[0] = 0xeb;
		buf[1] = offset-2;
		return insert_opcodes(buf, 2, buffer, size, 0);
	}
	else if (get_signed_nb_bits(offset - 5) <= 32) {
		if(size < sizeof(uint32_t)+1) return -1;
		// 32-bit offset (jmp rel32off E9)
		buf[0] = 0xe9;
		uint32_t off = offset-5;
		memcpy((void*)(buf+1), (void*)(&off), sizeof(uint32_t));
		return insert_opcodes(buf, sizeof(uint32_t)+1, buffer, size, 0);
	}
	else if (get_unsigned_nb_bits(reloc_addr) <= 32) { // 32-bits jump
		// mov eax, addr (B8 + rd); jump eax (FF E0)
		if(size < 7) return -1;
		buf[0] = 0xb8; // mov eax, addr
		uint32_t addr = reloc_addr;
		memcpy((void*)(buf+1), (void*)(&addr), sizeof(uint32_t));
		buf[5] = 0xff; // jmp eax
		buf[6] = 0xe0;
		return insert_opcodes(buf, 7, buffer, size, 0);
	} else { // 64-bits jump
		if(size < 12) return -1;
		// Absolute jump (mov rax, addr (REX.W + B8+ rd) ; jmp rax)
		buf[0] = 0x48; // mov rax, addr
		buf[1] = 0xb8;
		memcpy((void*)(buf+2), (void*)(&reloc_addr), sizeof(uint64_t));
		buf[10] = 0xff; // jmp rax
		buf[11] = 0xe0;
		return insert_opcodes(buf, 12, buffer, size, 0);
	}
}

