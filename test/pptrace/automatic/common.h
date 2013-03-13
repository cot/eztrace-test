/*
 * common.h - common basis for several tests
 *
 *  Created on: 3 sep. 2012
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <stdlib.h>
#include <stdio.h>

int debug;

// Common parts
#define DEBUG(x) if(debug > 1) fprintf(stderr, "\tDEBUG %s: " x "\n", __method__)
#define DEBUG1(x, p1) if(debug > 1) fprintf(stderr, "\tDEBUG %s: " x "\n", __method__, p1)
#define DEBUG2(x, p1, p2) if(debug > 1) fprintf(stderr, "\tDEBUG %s: " x "\n", __method__, p1, p2)
#define BEGIN_TEST(method) \
	 int do_run_test_##method; \
     int test_##method() { static const char *__method__ = #method; if(debug > 0) fprintf(stderr, "%s\n", __method__);
#define END_TEST(method) return 0; }
#define RUN_TEST(x) do { \
	if(runall || do_run_test_##x) { \
		r = test_##x(); \
		if(r < 0) { \
			return r; \
		} \
	} \
	} while(0)
#define PARSE_TEST(x) do { \
		if(strcmp(av[i], #x) == 0) { \
			do_run_test_##x = 1; \
			runall = 0; \
		} \
	} while(0)
#define TEST0(test, testname, msg) do { \
		if(!(test)) { \
			fprintf(stderr, "FAIL %s for test `" #testname "` with message: " msg "\n", __method__); \
			return -__LINE__; \
		} \
		DEBUG(#testname " succeeded"); \
	} while(0)
#define TEST1(test, testname, msg, p1) do { \
		if(!(test)) { \
			fprintf(stderr, "FAIL %s for test `" #testname "` with message: " msg "\n", __method__, p1); \
			return -__LINE__; \
		} \
		DEBUG(#testname " succeeded"); \
	} while(0)
#define TEST2(test, testname, msg, p1, p2) do { \
		if(!(test)) { \
			fprintf(stderr, "FAIL %s for test `" #testname "` with message: " msg "\n", __method__, p1, p2); \
			return -__LINE__; \
		} \
		DEBUG(#testname " succeeded"); \
	} while(0)
#define TEST3(test, testname, msg, p1, p2, p3) do { \
		if(!(test)) { \
			fprintf(stderr, "FAIL %s for test `" #testname "` with message: " msg "\n", __method__, p1, p2, p3); \
			return -__LINE__; \
		} \
		DEBUG(#testname " succeeded"); \
	} while(0)
// End of common parts
