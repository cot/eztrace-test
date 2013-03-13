/*
 * hijack.c - test for symbol hijacking
 *
 *  Created on: 4 Sep. 2012
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <errno.h>
#include <string.h>
#include <binary.h>
#include <tracing.h>
#include <hijack.h>
#include <opcodes.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include "common.h"

int testfork;
int waitattach;
// Common parts
pid_t child;
int fds[2];
void *bin;
char *prog;

inline int run_ipc() {
	if(testfork) return 0;
	pipe(fds);
	child = trace_run(NULL, NULL, NULL, 1);
	if(child > 0) {
		close(fds[1]);
		bin = open_binary(prog);
		return 1;
	}
	return 0;
}

inline void close_ipc() {
	close_binary(bin);
	close(fds[0]);
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
#define READ_WORD(w) TEST1(-1 != read(fds[0], &w, sizeof(w)), read, "read failed with error %s!", strerror(errno))
#define WRITE_WORD(w) write(fds[1], &w, sizeof(w))

void buffer_function() {
	__asm__("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop");
}
void bad_function() {
	char *__method__ = "bad_function";
	int r = 3;
	DEBUG("in");
	WRITE_WORD(r);
}
void (*original_pointer)() = bad_function;
void original_function() {
	char *__method__ = "original_function";
	int r = 1;
	DEBUG("in");
	WRITE_WORD(r);
}
void good_function() {
	char *__method__ = "good_function";
	int r = 2;
	DEBUG2("in (original_pointer = %p, r = %d)", original_pointer, r);
	WRITE_WORD(r);
	original_pointer();
	WRITE_WORD(r);
	DEBUG("out");
}
#define BEGIN_TEST_HIJACK(name) \
	BEGIN_TEST(name) \
		if(run_ipc()) {
#define END_TEST_HIJACK(name) \
			FINISH_IPC(); \
			int r; \
			READ_WORD(r); TEST1(r == 2, prolog, "Not in good_function() (r = %d)!", r); \
			READ_WORD(r); TEST1(r == 1, original, "Not in original_function() (r = %d)!", r); \
			READ_WORD(r); TEST1(r == 2, epilog, "Not in good_function() (r = %d)!", r); \
			close_ipc(); \
		} else { \
			if(waitattach) { fprintf(stderr, "child pid = %d\n", getpid()); sleep(10); } \
			original_function(); \
			IPC_END(); \
		} \
    END_TEST(name)
// End of common parts

BEGIN_TEST_HIJACK(hijack_code)
	ssize_t res = hijack_code(bin, child,
			(uint64_t)(word_int)original_function, (uint64_t)(word_int)good_function-(uint64_t)(word_int)original_function,
			(uint64_t)(word_int)buffer_function,
			(uint64_t)(word_int)&original_pointer, (uint64_t)(word_int)good_function);
	TEST1(res > 0, result, "hijack_code returned negative value ("WORD_DEC_FORMAT")!", res);
END_TEST_HIJACK(hijack_code)

BEGIN_TEST_HIJACK(hijack)
	INIT_ZZT_SYMBOL(toHijack, (word_int)original_function, (word_int)good_function-(word_int)original_function);
	INIT_ZZT_SYMBOL(orig,     (word_int)&original_pointer, sizeof(original_pointer));
	INIT_ZZT_SYMBOL(repl,     (word_int)good_function, 0);
	ssize_t res = hijack(bin, child, &toHijack, &orig, &repl);
	TEST1(res > 0, result, "hijack returned negative value ("WORD_DEC_FORMAT")!", res);
END_TEST_HIJACK(hijack)

void usage(char **av, char option) {
	if(option != 0) {
		fprintf(stderr, "Unknown option '%c'", option);
	}
	fprintf(stderr, "Usage: %s [-v[v]dg] [test1 test2 test3 ...]\n"
			"\t-v output verbose debugging information (the more d option, the more verbose the output)\n"
			"\t-d don't actually run the tests, simply execute the child (used in conjunction with test specifiers to debug)\n"
			"\t-g print the pid of the child and sleep 10 seconds after detachment to enable GDB attachment.\n"
			"\ttest specifies the tests to runs (default is all tests). Available tests: hijack_code, hijack.\n",
			av[0]);
	exit(-1);
}
int main(int ac, char **av) {
	int r, i, j;
	debug = 0;
	testfork = 0;
	waitattach = 0;
	int runall = 1;
	prog = av[0];
	for(i = 1; i < ac; i++) {
		if(av[i][0] == '-') {
			for(j = 1; av[i][j] != 0; j++) {
				switch(av[i][j]) {
				case 'v': debug++; break;
				case 'g': waitattach++; break;
				case 'd': testfork++; break;
				default: usage(av, av[i][j]);
				}
			}
		} else {
			PARSE_TEST(hijack_code);
			PARSE_TEST(hijack);
		}
	}

	RUN_TEST(hijack_code);
	RUN_TEST(hijack);

	return 0;
}
