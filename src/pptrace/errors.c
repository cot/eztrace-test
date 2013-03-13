/*
 * errors.c
 *
 *  Created on: 2 Aug. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <config.h>
#include <errors.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

char errmsg[1024];
int pptrace_debug_level = __PPTRACE_DEBUG_LEVEL;

const char* pptrace_get_error()
{
	const char *bin_error = pptrace_bin_error();
	if(bin_error)
		return bin_error;
	if(errmsg[0])
		return errmsg;
	if(errno) {
		return strerror(errno);
	}
	return NULL;
}

void pptrace_clear_error()
{
	errmsg[0] = 0;
}

void pptrace_report_error(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vsnprintf(errmsg, sizeof(errmsg), fmt, va);
	va_end(va);
}

void pptrace_error(char *fmt, ...)
{
#if PPTRACE_DEBUG_LEVEL_NONE <= __PPTRACE_DEBUG_LEVEL
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);

	const char *emsg = pptrace_get_error();
	if(emsg) {
		fprintf(stderr, ": %s\n", emsg);
	}
	else {
		fprintf(stderr, "\n");
	}
#endif
}

void pptrace_fubar(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "FUBAR: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
	exit(-1);
}

void pptrace_debug(int level, char *fmt, ...)
{
	if(level <= pptrace_debug_level) {
		va_list va;
		va_start(va, fmt);
		vfprintf(stderr, fmt, va);
		va_end(va);
	}
}

void pptrace_dump_buffer(int level, char *buffer, size_t length)
{
	int i;
	if(level <= pptrace_debug_level) {
		for(i = 0; i < length; i++) {
			fprintf(stderr, "%02x ", buffer[i]);
			if(i % 20 == 0) fprintf(stderr, "\n");
		}
		fprintf(stderr, "\n");
	}
}
