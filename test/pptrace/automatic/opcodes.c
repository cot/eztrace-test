/*
 * opcodes.c - tests for opcode generation
 *
 *  Created on: 3 Sep. 2012
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <string.h>
#include <binary.h>
#include <opcodes.h>
#include <tracing.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include "common.h"

#define MIN(a,b) (((a) > (b)) ? (b) : (a))

BEGIN_TEST(insert_opcodes)
	// Try to insert opcodes in a a buffer
	size_t size = 10;
	uint8_t buf[15];
	int i;
	for(i = 0; i < 15; i++)
		buf[i] = 0;

	ssize_t offset = 0;
	ssize_t prev = 0;
#define INSERT_OP_TEST(str, expected) do { \
		DEBUG2("inserting %s at offset "WORD_DEC_FORMAT, str, offset); \
		offset = insert_opcodes((uint8_t*)str, strlen(str), buf, size, offset); \
		buf[14] = 0; \
		TEST2(offset == MIN(prev + strlen(str), size), return, "returned "WORD_DEC_FORMAT" while awaiting "WORD_DEC_FORMAT"!", offset, MIN(prev + strlen(str), size)); \
		TEST2(strcmp(buf, expected) == 0, result, "buffer is `%s` while awaiting `%s`!", buf, expected); \
		prev = offset; \
	} while(0)

	INSERT_OP_TEST("ABC", "ABC");
	INSERT_OP_TEST("ABCDE", "ABCABCDE");
	INSERT_OP_TEST("ABCDE", "ABCABCDEAB");
END_TEST(insert_opcodes)

int test_function() {
	return 1;
}
BEGIN_TEST(generate_trampoline)
	void *maped_area;
	void *maped_area2;
    ssize_t offset;
	int (*f)(void);
#define GENTRAMP_TEST(options, off) do { \
		offset = 0; \
		maped_area = mmap(NULL, 1024, PROT_READ|PROT_WRITE|PROT_EXEC, options, -1, 0); \
		TEST0(maped_area != MAP_FAILED, set-up_##off, "mmap failed!"); \
		maped_area2 = (void*)((uint8_t*)maped_area + off); \
		offset = generate_trampoline(maped_area, off, (uint64_t)(word_int)maped_area, (uint64_t)(word_int)test_function); \
		TEST1(offset > 0, result_##off, "returned value from generate_trampoline is null or negative ("WORD_DEC_FORMAT")!", offset); \
		offset = generate_trampoline(maped_area2, 1024-off, (uint64_t)(word_int)maped_area2, (uint64_t)(word_int)maped_area); \
		TEST1(offset > 0, result_##off, "returned value from generate_trampoline is null or negative ("WORD_DEC_FORMAT")!", offset); \
		f = (int (*)(void))maped_area2; \
		int r = f(); \
		TEST1(f() == 1, jump_##off, "jump returned %d while awaiting 1!", r); \
		munmap(maped_area, 1024); \
	} while(0)
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE, 512);
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE, 128);
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE, 127);
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE, 126);
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE, 125);
#if __WORDSIZE == 64
	// Trying 32-bits mapping in 64 bits environment
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, 512);
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, 128);
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, 127);
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, 126);
	GENTRAMP_TEST(MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, 125);
#endif
END_TEST(generate_trampoline)

BEGIN_TEST(read_jump)
	uint8_t buffer[] = {
		// from testcase/test.cc main function
		0x55, //                   	push   %rbp
		0x48, 0x89, 0xe5, //            	mov    %rsp,%rbp
		0x48, 0x83, 0xec, 0x10, //         	sub    $0x10,%rsp
		0xc7, 0x45, 0xf8, 0x00, 0x00, 0x00, 0x00, //	movl   $0x0,-0x8(%rbp)
		0xeb, 0x34, //               	jmp   , 0x400539 <main+0x45>
		0xc7, 0x45, 0xfc, 0x00, 0x00, 0x00, 0x00, //	movl   $0x0,-0x4(%rbp)
		0xeb, 0x1c, //               	jmp   , 0x40052a <main+0x36>
		0x8b, 0x45, 0xf8, //            	mov    -0x8(%rbp),%eax
		0x0f, 0xaf, 0x45, 0xfc, //         	imul   -0x4(%rbp),%eax
		0x89, 0xc6, //               	mov    %eax,%esi
		0xbf, 0x3c, 0x06, 0x40, 0x00, //      	mov    $0x40063c,%edi
		0xb8, 0x00, 0x00, 0x00, 0x00, //      	mov    $0x0,%eax
		0xe8, 0xca, 0xfe, 0xff, 0xff, //      	callq , 0x4003f0 <printf@plt>
		0x83, 0x45, 0xfc, 0x01, //         	addl   $0x1,-0x4(%rbp)
		0x83, 0x7d, 0xfc, 0x13, //         	cmpl   $0x13,-0x4(%rbp)
		0x0f, 0x9e, 0xc0, //            	setle  %al
		0x84, 0xc0, //               	test   %al,%al
		0x75, 0xd9, //               	jne   , 0x40050e <main+0x1a>
		0x83, 0x45, 0xf8, 0x01, //         	addl   $0x1,-0x8(%rbp)
		0x83, 0x7d, 0xf8, 0x09, //         	cmpl   $0x9,-0x8(%rbp)
		0x0f, 0x9e, 0xc0, //            	setle  %al
		0x84, 0xc0, //               	test   %al,%al
		0x75, 0xc1, //               	jne    400505 <main+0x11>
		0xb8, 0x00, 0x00, 0x00, 0x00, //      	mov    $0x0,%eax
		0xc9, //                   	leaveq
		0xc3, //                   	retq
	};
	int sizes[] = { 1, 3, 4, 7, 2, 7, 2, 3, 4, 2, 5, 5, 5, 4, 4, 3, 2, 2, 4, 4, 3, 2, 2, 5, 1, 1, 0 };
	int jumps[] = { 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1 };
	int i;
	size_t offset = 0;
	// the actual test
	for(i = 0; sizes[i] != 0; i++) {
		/*  - read_jump(buf, size, offset, prefix): read a jump part that was prefixed
		 *  	by *prefix* and that is contained at offset *offset* of *buf* (*size* is
		 *  	the size of *buf*). The returned value is the size of the instruction
		 *  	if a jump is read, 0 if a valid prefix for a jump was encountered, and < 0
		 *  	if the instruction is not a jump.
		 */
		ssize_t r = read_jump(buffer, 87, offset, 0);
		if(r == 0) {
			r = read_jump(buffer, 87, offset+1, buffer[offset]);
			if(r > 0) r++;
		}
		if(r < 0) {
			TEST1(jumps[i] == 0, notjump, "instruction %d was a jump but wasn't detected!", i);
		} else {
			TEST2(jumps[i] == 1, jump, "instruction %d wasn't a jump but was detected as one with size "WORD_DEC_FORMAT"!", i, r);
			TEST3(sizes[i] == r, size, "instruction %d has size %d but got "WORD_DEC_FORMAT"!", i, sizes[i], r);
		}
		offset += sizes[i];
	}
END_TEST(read_jump)

void usage(char **av, char option) {
	if(option != 0) {
		fprintf(stderr, "Unknown option '%c'", option);
	}
	fprintf(stderr, "Usage: %s [-v[v]] [test1 test2 test3 ...]\n"
			"\t-v output verbose debugging information (the more d option, the more verbose the output)\n"
			"\ttest specifies the tests to runs (default is all tests). Available tests: insert_opcodes, generate_trampoline, read_jump.\n",
			av[0]);
	exit(-1);
}
int main(int ac, char **av) {
	int r, i, j;
	debug = 0;
	int runall = 1;
	for(i = 1; i < ac; i++) {
		if(av[i][0] == '-') {
			for(j = 1; av[i][j] != 0; j++) {
				switch(av[i][j]) {
				case 'v': debug++; break;
				default: usage(av, av[i][j]);
				}
			}
		} else {
			PARSE_TEST(insert_opcodes);
			PARSE_TEST(generate_trampoline);
			PARSE_TEST(read_jump);
		}
	}

	RUN_TEST(insert_opcodes);
	RUN_TEST(generate_trampoline);
	RUN_TEST(read_jump);

	return 0;
}
