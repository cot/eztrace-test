/*
 * isize.c
 *
 * Wrapper to include the good library for determining the instruction size
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <config.h>
#include <binary.h>

#ifndef __PPTRACE_ISIZE_TYPE
#include "isize/trace.c"
#else

#if __PPTRACE_ISIZE_TYPE == PPTRACE_ISIZE_TYPE_OPCODE
#include "isize/opcode.c"
#else // if ! (__PPTRACE_ISIZE_TYPE == PPTRACE_ISIZE_TYPE_OPCODE)
#include "isize/trace.c"
#endif // ! (__PPTRACE_ISIZE_TYPE == PPTRACE_ISIZE_TYPE_OPCODE)

#endif // !defined(__PPTRACE_ISIZE_TYPE)
