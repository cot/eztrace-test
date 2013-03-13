/*
 * errors.h -- Error reporting inside PPTrace.
 *
 * Methods provided are:
 * 	- pptrace_get_error(): returns the last reported error
 * 	- pptrace_clear_error(): clear the error reported by PPTrace internals
 * 	- pptrace_error(fmt, ...): printf-like function to display an error.
 * 			*PPTrace Internals*
 *	- pptrace_report_error(fmt, ...): printf-like function to report an error
 *			that won't be displayed. *PPTrace Internals*
 *	- pptrace_fubar(fmt, ...): printf-like function for fatal error. This ends
 *			the process. *PPTrace Internals*
 *	- pptrace_bin_error(): method implemented by the binary module to provides
 *			error specific to the binary module. *PPTrace Internals*
 *	- pptrace_debug(level, fmt, ...): printf-like function for printing
 *			debugging information. *PPTrace Internals*
 *	- pptrace_dump_buffer(level, buffer, length): print in hex format the
 *			buffer given in argument for debugging purpose. *PPTrace Internals*
 *
 *  Created on: 2 Aug. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#ifndef PPTRACE_ERRORS_H_
#define PPTRACE_ERRORS_H_

#include <config.h>
#include <unistd.h>

const char* pptrace_get_error();
void pptrace_clear_error();
void pptrace_error(char *fmt, ...);

void pptrace_report_error(char *fmt, ...);
void pptrace_fubar(char *fmt, ...);

const char* pptrace_bin_error();

void pptrace_debug(int level, char *fmt, ...);
void pptrace_dump_buffer(int level, char *buffer, size_t length);

#endif // PPTRACE_ERRORS_H_
