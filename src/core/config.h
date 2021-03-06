/*
 * config.h -- Configuration parameters
 *
 *  Created on: 02 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 *
 */

#ifndef PPTRACE_CONFIG_H_
#define PPTRACE_CONFIG_H_

#define PPTRACE_DEBUG_LEVEL_NONE    0
#define PPTRACE_DEBUG_LEVEL_INFO    1
#define PPTRACE_DEBUG_LEVEL_VERBOSE 2
#define PPTRACE_DEBUG_LEVEL_DEBUG   3
#define PPTRACE_DEBUG_LEVEL_ALL     4

#define PPTRACE_BINARY_TYPE_BFD 1  // Requires -lbfd
#define PPTRACE_BINARY_TYPE_ELF 2  // Requires -lelf

#define PPTRACE_ARCH_TYPE_INTEL 1 // What else?

#define PTRACE_TRACE_TYPE_LINUX  1
#define PTRACE_TRACE_TYPE_MACOSX 2 // Not Yet Implemented
#define PTRACE_TRACE_TYPE_BSD    3 // Not Yet Implemented

#define PPTRACE_ISIZE_TYPE_TRACE  1
#define PPTRACE_ISIZE_TYPE_OPCODE 2 // Requires -lopcodes and binary type bfd

// Configuration
 
// Syscall dependency

#ifndef __PPTRACE_DEBUG_LEVEL
#define __PPTRACE_DEBUG_LEVEL 0
#endif // defined __PPTRACE_DEBUG_LEVEL

#ifndef __PPTRACE_BINARY_TYPE
#define __PPTRACE_BINARY_TYPE 1
#endif // defined __PPTRACE_BINARY_TYPE


#define HAVE_PRCTL_H 1
#ifndef  __PPTRACE_USE_PRCTL
#if HAVE_PRCTL_H
// This flag is set to use prctl for ptrace to allow child to trace its parent on hardened systems
// Set it if prtcl.h, the prtcl() function and the PR_SET_PTRACER constant exists
#define __PPTRACE_USE_PRCTL
#endif /* HAVE_PRCTL_H */
#endif // defined  __PPTRACE_USE_PRCTL

#ifndef __PPTRACE_ARCH_TYPE
#define __PPTRACE_ARCH_TYPE PPTRACE_ARCH_TYPE_INTEL
#endif // defined __PPTRACE_ARCH_TYPE

#ifndef __PPTRACE_TRACE_TYPE
#define __PPTRACE_TRACE_TYPE PTRACE_TRACE_TYPE_LINUX
#endif // defined __PPTRACE_TRACE_TYPE

#ifndef __PPTRACE_ISIZE_TYPE
#define __PPTRACE_ISIZE_TYPE PPTRACE_ISIZE_TYPE_TRACE
#endif // defined __PPTRACE_ISIZE_TYPE

#if (__PPTRACE_ISIZE_TYPE == PPTRACE_ISIZE_TYPE_OPCODE) && (__PPTRACE_BINARY_TYPE != PPTRACE_BINARY_TYPE_BFD)
#undef __PPTRACE_ISIZE_TYPE
#define __PPTRACE_ISIZE_TYPE PPTRACE_ISIZE_TYPE_TRACE
#endif // (__PPTRACE_ISIZE_TYPE == PPTRACE_ISIZE_TYPE_OPCODE) && (__PPTRACE_BINARY_TYPE != PPTRACE_BINARY_TYPE_BFD)

// Obscure internal structures
#define PPTRACE_HIJACK_FUNCTION       __pptrace_hijack_list
#define PPTRACE_HIJACK_FUNCTION_NAME "__pptrace_hijack_list"

#endif /* PPTRACE_CONFIG_H_ */
