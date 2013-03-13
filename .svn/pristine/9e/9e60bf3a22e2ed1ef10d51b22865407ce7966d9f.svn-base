/*
 * tracing.c - test for tracing functions
 *
 *  Created on: 26 Aug. 2012
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <string.h>
#include <binary.h>
#include <tracing.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include "common.h"

int testfork;

// Common parts
#define instr_call_parent() __asm__("int3")
#define call_parent() \
	do { \
		DEBUG("traced process is before call_parent"); \
		instr_call_parent(); \
		DEBUG("traced process is after call_parent"); \
	} while(0)
#define call_parent_lbl(lbl) \
	do { \
		DEBUG("traced process is before call_parent"); \
		instr_call_parent(); \
		do_lbl(lbl); \
	} while(0)
#define lbl_name(lbl) test_##lbl
#define lbl_addr(lbl) &&lbl_name(lbl)
#define do_lbl(lbl) do { \
		lbl_name(lbl): \
		DEBUG("traced process is after label " #lbl); \
	} while(0)

pid_t child;
int fds[2];

inline int run_ipc() {
	if(testfork) return 0;
	pipe(fds);
	child = trace_run(NULL, NULL, NULL, 1);
	return child > 0 ? 1 : 0;
}
inline void close_ipc() {
	close(fds[0]);
	close(fds[1]);
}
#define FINISH_IPC() do { \
		trace_detach(child); \
		trace_wait(child); \
		DEBUG("process finished"); \
	} while(0)
#define IPC_END() do { \
		DEBUG("traced process is exiting"); \
		exit(0); \
	} while(0)
#define READ_WORD(w) read(fds[0], &w, sizeof(w))
#define WRITE_WORD(w) write(fds[1], &w, sizeof(w))

// End of common parts
BEGIN_TEST(getset_ip)
	void* r;
	word_int rr;
	// print the ip of the int 3 instruction and propose a function ip
	if(run_ipc()) {
		// The tracing process
		trace_continue(child);
		r = (void*)get_ip(child);
		TEST2(lbl_addr(getset_ip_lbl1) == r, get_ip, "returned %p while awaiting %p!", r, lbl_addr(getset_ip_lbl1));
		set_ip(child, (word_int)lbl_addr(getset_ip_lbl2));
		FINISH_IPC();
		READ_WORD(rr);
		TEST1(rr == 1, set_ip, "returned "WORD_DEC_FORMAT" while awaiting 1!", rr);
		close_ipc();
	} else {
		// The traced process
		rr = 1;
		call_parent_lbl(getset_ip_lbl1);
		rr = 0;
		do_lbl(getset_ip_lbl2);
		WRITE_WORD(rr);
		IPC_END();
	}
END_TEST(getset_ip)

BEGIN_TEST(trace_readwrite)
	static word_int result; 	// try write then read result and compares
	if(run_ipc()) {
		word_int rr;
		trace_continue(child);
		READ_WORD(result);
		trace_read(child, (word_int)&result, (uint8_t*)&rr, sizeof(rr));
		TEST2(rr == result, trace_read, "returned "WORD_HEX_FORMAT" while awaiting "WORD_HEX_FORMAT"!", rr, result);
		rr = 42;
		trace_write(child, (word_int)&result, (uint8_t*)&rr, sizeof(rr));
		FINISH_IPC();
		READ_WORD(result);
		TEST2(rr == result, trace_write, "trace_write(): value of result is "WORD_HEX_FORMAT" while awaiting "WORD_HEX_FORMAT"!", rr, result);
		close_ipc();
	} else {
		result = 4242;
		WRITE_WORD(result);
		call_parent();
		WRITE_WORD(result);
		IPC_END();
	}
END_TEST(trace_readwrite)

BEGIN_TEST(trace_replace)
	// give one address and the original value and a value to replace
	static uint8_t value;
	if(run_ipc()) {
		trace_continue(child);
		READ_WORD(value);
		uint8_t rr = trace_replace(child, (word_int)&value, 42);
		TEST2(value == rr, trace_replace, "returned %x while awaiting %x", rr, value);
		FINISH_IPC();
		READ_WORD(value);
		TEST1(value == 42, trace_replace, "written %x while awaiting 42", value);
		close_ipc();
	} else {
		value = 24;
		WRITE_WORD(value);
		call_parent();
		WRITE_WORD(value);
		IPC_END();
	}
END_TEST(trace_replace)

BEGIN_TEST(trace_copy)
	// give two addresses and a string then print the resulting string
	static char buffer[10];
	static const char *tocopy = "BBBBBBBBB\0ABC";
	word_int val = -1;
	if(run_ipc()) {
		trace_copy(child, (word_int)tocopy, (word_int)buffer, strlen(tocopy)+1);
		FINISH_IPC();
		READ_WORD(val);
		TEST0(val == 1, trace_copy, "the buffer comparison failed");
		close_ipc();
	} else {
		val = strcmp(buffer, tocopy) == 0 ? 1 : 0;
		if(!val) {
			DEBUG2("%s != %s!", buffer, tocopy);
		}
		WRITE_WORD(val);
		IPC_END();
	}
END_TEST(trace_copy)

BEGIN_TEST(trace_singlestep)
	void *r;
	if(run_ipc()) {
		trace_continue(child);
		r = (void*)get_ip(child);
		TEST2(lbl_addr(singlestep_lbl1) == r, set-up, "eip is %p while awaiting %p!", r, lbl_addr(singlestep_lbl1));
		trace_singlestep(child);
		r = (void*)get_ip(child);
		TEST2(lbl_addr(singlestep_lbl2) == r, singlestep, "eip is %p while awaiting %p!", r, lbl_addr(singlestep_lbl2));
		FINISH_IPC();
		close_ipc();
	} else {
		instr_call_parent();
		lbl_name(singlestep_lbl1):
#if __WORDSIZE == 64
		__asm__("inc %rax");
#else
		__asm__("inc %eax");
#endif
		lbl_name(singlestep_lbl2):
		IPC_END();
	}
END_TEST(trace_singlestep)

BEGIN_TEST(trace_mmap)
	static char *buf1 = "ABCDEF";
	static char *buf2;
	if(run_ipc()) {
		word_int mmap = trace_mmap(child, 0, 1024, PROT_READ|PROT_WRITE);
		TEST0(mmap != 0, mmap_nonnull, "mmap returned null!");
		int errcode = (int)(-(signed_word)mmap);
		TEST2(errcode < 0 || errcode > 127, mmap_failed, "mmap returned the error code %d (strerror = %s)!", errcode, strerror(errcode));
		DEBUG1("trace_mmap returned %p", (void*)mmap);
		trace_write(child, (word_int)&buf2, (uint8_t*)&mmap, sizeof(mmap));
		DEBUG("written the address of mmap to buf2");
		trace_copy(child, (word_int)buf1, mmap, strlen(buf1)+1);
		DEBUG("copied content of buf1 to mmap'd content");
		FINISH_IPC();
		READ_WORD(mmap);
		TEST0(mmap == 1, mmap_copy, "data comparison failed!");
		close_ipc();
	} else {
		DEBUG1("buf2 = %p", buf2);
		word_int val = strcmp(buf1, buf2) == 0 ? 1 : 0;
		if(!val) {
			DEBUG2("%s != %s!", buf1, buf2);
		}
		WRITE_WORD(val);
		IPC_END();
	}
END_TEST(trace_mmap)

void usage(char **av, char option) {
	if(option != 0) {
		fprintf(stderr, "Unknown option '%c'", option);
	}
	fprintf(stderr, "Usage: %s [-v[v]] [test1 test2 test3 ...]\n"
			"\t-v output verbose debugging information (the more d option, the more verbose the output)\n"
			"\t-d don't actually run the tests, simply execute the child (used in conjunction with test specifiers to debug)\n"
			"\ttest specifies the tests to runs (default is all tests). Available tests: getset_ip, readwrite, replace, copy, singlestep, mmap.\n",
			av[0]);
	exit(-1);
}
int main(int ac, char **av) {
	int r, i, j;
	debug = 0;
	testfork = 0;
	int runall = 1;
	for(i = 1; i < ac; i++) {
		if(av[i][0] == '-') {
			for(j = 1; av[i][j] != 0; j++) {
				switch(av[i][j]) {
				case 'v': debug++; break;
				case 'd': testfork++; break;
				default: usage(av, av[i][j]);
				}
			}
		} else {
			PARSE_TEST(getset_ip);
			PARSE_TEST(trace_readwrite);
			PARSE_TEST(trace_replace);
			PARSE_TEST(trace_copy);
			PARSE_TEST(trace_singlestep);
			PARSE_TEST(trace_mmap);
		}
	}

	RUN_TEST(getset_ip);
	RUN_TEST(trace_readwrite);
	RUN_TEST(trace_replace);
	RUN_TEST(trace_copy);
	RUN_TEST(trace_singlestep);
	RUN_TEST(trace_mmap);

	return 0;
}
