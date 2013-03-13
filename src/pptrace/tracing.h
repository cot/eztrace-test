/*
 * tracing.h -- API for tracing process using ptrace() or similar
 *
 * - trace_run(path, argv, envp, debug): executes a process and trace it
 * 		Arguments should be the same as execve. The returned value
 * 		is the pid of the traced process.
 * 		If path is NULL, the process is only forked and trace_run
 * 		act as fork, the traced process return 0 and the tracing
 * 		returned the pid of the traced process. debug tells
 * 		the trace_run function that we would like to debug at the
 * 		detachment of the process (so run the program in background).
 * - trace_detach(child): detach (stop tracing) the process *child*.
 * - trace_wait(child): wait for the end of the process *child*.
 * - trace_set_bits(bits): tells the tracing system the size of a word
 * 	in the trace process (default is the wordsize of the host process).
 * - get_ip(child): returns the instruction pointer of process *child*.
 * - set_ip(child, rip): set to *rip* the instruction pointer of process
 * 			*child*.
 * - trace_read(child, fromaddr, buffer, length): reads *length* bytes
 * 		into *buffer* from *child* memory starting at address *fromaddr*.
 * - trace_write(child, fromaddr, buffer, length): write *length* bytes
 * 		from *buffer* to *child* memory starting at address *fromaddr*.
 * - trace_replace(child, addr, newbyte): set the byte at address *addr*
 * 		of *child* memory to *newbyte*. The previous value of this address
 * 		is returned by this function.
 * - trace_copy(child, fromaddr, toaddr, length): copies *length* bytes
 * 		from address *fromaddr* to address *toaddr* of *child* memory.
 * - trace_singlestep(child): executes one instruction of *child* and
 * 		stops after.
 * - trace_continue(child): Continue the child until a signal is caught.
 * - trace_syscall(child): Continue the child until a signal is caught or
 * 		a syscall is issued.
 * 	- trace_mmap(child, addr, length, prot): do, inside *child*, a
 * 	  mmap(addr, length, prot, MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT, -1, 0).
 * 	    This function return the result of the syscall.
 * 	- trace_wait_syscall(child, retval, syscall, ...): wait for syscall number
 * 		*syscall* to happen in *child*.  Result of the syscall is store in *retval*.
 * 		The arguments after are a series of SYSCALL_ARGTYPE_* ended
 * 		by SYSCALL_ARGTYPE_END.
 * 			SYSCALL_ARGTYPE_INT waits for an integer that matches the next
 * 				argument
 * 			SYSCALL_ARGTYPE_ZSTRING waits for a null terminated string that
 * 				matches the next argument
 * 			SYSCALL_ARGTYPE_ZSTRING waits for a null terminated string that
 * 				will be returned in the pointer given as next argument
 * 			SYSCALL_ARGTYPE_IGNORE waits for any argument
 *      trace_wait_syscall returns < 0 when the wait did not encountered
 *      the awaited syscall and 0 on success.
 *      For use with several similar syscalls, multiple syscall numbers can be fit
 *      together in syscall using high bits range (2 byte per syscall number)
 * 	- trace_get_regs(pid_t child, REG_STRUCT* regs): returns the registers
 * 		of process *child* into *regs*.
 *  - trace_set_regs(pid_t child, REG_STRUCT* regs): set the registers of
 *  	process *child* to be *regs*.
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#ifndef PPTRACE_TRACING_H_
#define PPTRACE_TRACING_H_

#include <stdint.h>
#include <sys/types.h>
#include <config.h>

//#ifndef __PPTRACE_TRACE_TYPE
#include <tracing/linux.h>
//#else

//#if __PPTRACE_TRACE_TYPE == PPTRACE_TRACE_TYPE_BSD
//#include <tracing/bsd.h>
//#elif __PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_MACOSX
//#include <tracing/macosx.h>
//#else // if ! (__PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_MACOSX || __PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_BSD)
//#include <tracing/linux.h>
//#endif // ! (__PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_MACOSX || __PPTRACE_TRACE_TYPE == PPTRACE_ARCH_TYPE_BSD)

//#endif // !defined(__PPTRACE_TRACE_TYPE)

#define SYSCALL_ARGTYPE_END            0
#define SYSCALL_ARGTYPE_INT            1
#define SYSCALL_ARGTYPE_ZSTRING        2
#define SYSCALL_ARGTYPE_ZSTRING_RETURN 3 // Return the value of the string
#define SYSCALL_ARGTYPE_IGNORE         4

pid_t trace_run(char *path, char **argv, char **envp, int debug);
void trace_detach(pid_t child);
void trace_wait(pid_t child);

void trace_set_bits(int bits);
word_int get_ip(pid_t child);
void set_ip(pid_t child, word_int rip);
void trace_read(pid_t child, word_int fromaddr, uint8_t* buffer, size_t length);
void trace_write(pid_t child, word_int addr, uint8_t* buffer, size_t length);
uint8_t trace_replace(pid_t child, word_int addr, uint8_t newbyte);
void trace_copy(pid_t child, word_int fromaddr, word_int toaddr, size_t length);
int trace_singlestep(pid_t child);
int trace_syscall(pid_t child);
int trace_continue(pid_t child);
word_int trace_mmap(pid_t child, word_int addr, size_t length, int prot);
int trace_wait_syscall(pid_t child, word_int *retval, unsigned long syscall, ...); // Wait for syscall and return the return value of the first encountered syscall
void trace_get_regs(pid_t child, REG_STRUCT* regs);
void trace_set_regs(pid_t child, REG_STRUCT* regs);

#endif /* PPTRACE_TRACING_H_ */
