/*
 * tracing.c
 *
 * Wrapper to include the good library for tracing processes
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <config.h>
#include <binary.h>
#include <stdlib.h>
#include <errors.h>
#include <tracing.h>

//#ifndef __PPTRACE_TRACE_TYPE
#include "tracing/linux.c"
//#else

//#if __PPTRACE_TRACE_TYPE == PPTRACE_TRACE_TYPE_BSD
//#include "tracing/bsd.c"
//#elif __PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_MACOSX
//#include "tracing/macosx.c"
//#else // if ! (__PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_MACOSX || __PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_BSD)
//#include "tracing/linux.c"
//#endif // ! (__PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_MACOSX || __PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_BSD)

//#endif // !defined(__PPTRACE_TRACE_TYPE)
