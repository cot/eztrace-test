/*
 * linux.c
 *
 * Implementation of the tracing interface using the linux _ptrace system call
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#ifdef __PPTRACE_USE_PRCTL
#include <sys/prctl.h>
#endif // __PPTRACE_USE_PRCTL
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errors.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#define CONTSIGNAL NULL
static int ptrace_bits = __WORDSIZE; // Size of a word in the traced process

#ifndef MAP_32BIT
#define MAP_32BIT 0
#endif

#if __WORDSIZE == 64
// Registers name
#define REG_IP rip
#define REG_AX rax
#define REG_ORIG_AX orig_rax
#define REG_BX rbx
#define REG_CX rcx
#define REG_DX rdx
#define REG_SI rsi
#define REG_DI rdi
#define REG_BP rbp
#define REG_R10 r10
#define REG_R8 r8
#define REG_R9 r9

// Debugging
#define FORMAT_WORD_HEX "%02lx %02lx %02lx %02lx %02lx %02lx %02lx %02lx"
#define FORMAT_WORD_POINTER "0x%016lx"
#define ARG_FORMAT_WORD(data) (data) & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff, (data >> 24) & 0xff, (data >> 32) & 0xff,\
		(data >> 40) & 0xff, (data >> 48) & 0xff, (data >> 56) & 0xff
#define REG_FORMAT "rip=0x%016lx rax=0x%016lx orig_rax=0x%016lx rbx=0x%016lx rcx=0x%016lx rdx=0x%016lx rsi=0x%016lx rdi=0x%016lx rbp=0x%016lx r10=0x%016lx r8=0x%016lx r9=0x%016lx"
#define REG_FORMAT_VALUES(regs) regs.rip, regs.rax, regs.orig_rax, regs.rbx, regs.rcx, regs.rdx, regs.rsi, regs.rdi, regs.rbp, regs.r10, regs.r8, regs.r9

#else  // if __WORDSIZE != 64
// Register names
#define REG_IP eip
#define REG_AX eax
#define REG_ORIG_AX orig_eax
#define REG_BX ebx
#define REG_CX ecx
#define REG_DX edx
#define REG_SI esi
#define REG_DI edi
#define REG_BP ebp
#define REG_R10 edi // dummy, doesn't exist in 32bits
#define REG_R8 esi // dummy, doesn't exist in 32bits
#define REG_R9 ebp // dummy, doesn't exist in 32bits
#define REG_FORMAT "eip=0x%08lx eax=0x%08lx orig_eax=0x%08lx ebx=0x%08lx ecx=0x%08lx edx=0x%08lx esi=0x%08lx edi=0x%08lx ebp=0x%08lx"
#define REG_FORMAT_VALUES(regs) regs.eip, regs.eax, regs.orig_eax, regs.ebx, regs.ecx, regs.edx, regs.esi, regs.edi, regs.ebp

// Debugging
#define FORMAT_WORD_HEX "%02lx %02lx %02lx %02lx"
#define FORMAT_WORD_POINTER "0x%08lx"
#define ARG_FORMAT_WORD(data) (data) & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff, (data >> 24) & 0xff

#endif //  __WORDSIZE == 64

void trace_set_bits(int bits)
{
	ptrace_bits = bits;
}

int trace_get_bits()
{
	return ptrace_bits;
}

/*
 * A wrapper to test for errors
 * TODO: Correct error handling in that part of the code
 */
inline long __ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data, const char *file, int line)
{
	errno = 0;
	long result = ptrace(request, pid, addr, data);
	if(errno && result == -1) {
		pptrace_fubar("at %s:%d: ptrace(%lu, %d, %p, %p): %s", file, line, (long)request, pid, addr, data, strerror(errno));
	}
	return result;
}
#define _ptrace(req, pid, addr, data) __ptrace(req, pid, addr, data, __FILE__, __LINE__)

extern int pptrace_debug_level;

pid_t trace_run(char *path, char **argv, char **envp, int debug)
{
	int status;
	int fds[2];
	pipe(fds);
	char plop;
	pid_t child = fork();

	if(pptrace_debug_level > PPTRACE_DEBUG_LEVEL_ALL)
		debug = 1;

	if((debug == 0 && child != 0) || (debug && child == 0)) {
#ifdef __PPTRACE_USE_PRCTL
#ifdef PR_SET_PTRACER
		prctl(PR_SET_PTRACER, child, 0, 0, 0);
#endif // PR_SET_PTRACER
#endif // __PPTRACE_USE_PRCTL
		if(child == 0)
			_ptrace(PTRACE_TRACEME, 0, NULL, NULL);

		read(fds[0], &plop, sizeof(char));
		close(fds[0]);
		if(path == NULL) { // No execution is asked (fork only) so return to the calling function
			kill(getpid(), SIGTRAP); // signal the parent to emulate execve call.
			return 0;
		}
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "execve(%s, ...)\n", path);
		execve(path, argv, envp);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "Execution of %s failed\n", path);
		exit(-1);
	}
	pid_t tpid = debug ? child : getppid();
	if(debug == 0)
		_ptrace(PTRACE_ATTACH, tpid, NULL, NULL);
	plop = 'a';
	write(fds[1], &plop, sizeof(char));
	close(fds[1]);
	waitpid(tpid, &status, 0);
	while(!WIFEXITED(status) && (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGTRAP)) {
		_ptrace(PTRACE_CONT, tpid, NULL, NULL);
		waitpid(tpid, &status, 0);
	}
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "execve(%s, ...) passed\n", path);
	if(WIFEXITED(status)) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "Process %d exited (probably due to execution of %s failed)\n", tpid, path);
		return -1;
	}
	return tpid;
}

word_int get_ip(pid_t child)
{
	REG_STRUCT regs;
	_ptrace(PTRACE_GETREGS, child, NULL, &regs);
	return regs.REG_IP;
}

void set_ip(pid_t child, word_int rip)
{
	REG_STRUCT regs;
	_ptrace(PTRACE_GETREGS, child, NULL, &regs);
	regs.REG_IP = rip;
	_ptrace(PTRACE_SETREGS, child, NULL, &regs);
}

uint8_t trace_replace(pid_t child, word_int addr, uint8_t newbyte)
{
	int wordsize = ptrace_bits / 8;
	word_int addrmask = ~((word_int)(wordsize-1));
	int shift         = (8*(addr%wordsize));

	word_int data64   = 0;
	word_int addr64   = addr & addrmask;
	word_int mask     = ~(((word_int)0xff) << shift);
	word_int newvalue = newbyte;
	word_int oldvalue = 0;
	newvalue <<= shift;

	data64 = _ptrace(PTRACE_PEEKTEXT, child, (void*)addr64, NULL);
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Replacing [" FORMAT_WORD_HEX "] from address " FORMAT_WORD_POINTER " by ", ARG_FORMAT_WORD(data64), addr64);

	oldvalue = data64 >> shift;
	oldvalue &= 0xff;

	data64 = (data64 & mask) | newvalue;
	pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "[" FORMAT_WORD_HEX "]\n", ARG_FORMAT_WORD(data64));
	_ptrace(PTRACE_POKETEXT, child, (void*)addr64, (void*)data64);

	return (uint8_t)oldvalue;
}

void trace_read(pid_t child, word_int fromaddr, uint8_t* buffer, size_t length)
{
	word_int data;
	word_int curAddr;

	for(curAddr = 0; curAddr < length; curAddr += 8) {
		data = _ptrace(PTRACE_PEEKTEXT, child, (void*)(curAddr+fromaddr), NULL);
		memcpy(buffer+curAddr, (void*)(&data), curAddr+8 < length ? 8 : (length-curAddr));
	}
}

void trace_copy(pid_t child, word_int fromaddr, word_int toaddr, size_t length)
{
	word_int data;
	word_int curAddr;
	int extraBytes = length % sizeof(word_int);

	pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Copying buffer of %ld bytes from address " FORMAT_WORD_POINTER " to address " FORMAT_WORD_POINTER "\n", length, fromaddr, toaddr);

	for(curAddr = 0; curAddr < length - extraBytes; curAddr += sizeof(word_int)) {
		data = _ptrace(PTRACE_PEEKTEXT, child, (void*)(curAddr+fromaddr), NULL);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Copying word [" FORMAT_WORD_HEX "] from address " FORMAT_WORD_POINTER " to address " FORMAT_WORD_POINTER "\n",
				ARG_FORMAT_WORD(data), curAddr+fromaddr, curAddr+toaddr);
		_ptrace(PTRACE_POKETEXT, child, (void*)(curAddr+toaddr), (void*)data);
	}

	if(extraBytes > 0) {
		extraBytes <<= 3; // in bits
		word_int mask = ((1L << (extraBytes))-1);
		data = _ptrace(PTRACE_PEEKTEXT, child, (void*)(curAddr+fromaddr), NULL);
		data &= mask;
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Copying %d extra bytes (word [" FORMAT_WORD_HEX "] from address " FORMAT_WORD_POINTER " to address " FORMAT_WORD_POINTER ")\n",
				extraBytes >> 3, ARG_FORMAT_WORD(data), curAddr+fromaddr, curAddr+toaddr);
		word_int destData = _ptrace(PTRACE_PEEKTEXT, child, (void*)(curAddr+toaddr), NULL);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Replacing [" FORMAT_WORD_HEX "] by ", ARG_FORMAT_WORD(destData));
		destData &= ~(mask);
		destData |= data;
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "[" FORMAT_WORD_HEX "]\n", ARG_FORMAT_WORD(destData));
		_ptrace(PTRACE_POKETEXT, child, (void*)(curAddr+toaddr), (void*)destData);
	}
}

int trace_singlestep(pid_t child)
{
	int status = 0;
	_ptrace(PTRACE_SINGLESTEP, child, NULL, NULL);
	waitpid(child, &status, 0);
	return status;
}

int trace_syscall(pid_t child)
{
	int status = 0;
	_ptrace(PTRACE_SYSCALL, child, NULL, NULL);
	waitpid(child, &status, 0);
	return status;
}

int trace_continue(pid_t child)
{
	int status = 0;
	_ptrace(PTRACE_CONT, child, NULL, NULL);
	waitpid(child, &status, 0);
	return status;
}

void trace_write(pid_t child, word_int addr, uint8_t* buffer, size_t length)
{
	word_int data;
	word_int curAddr;
	int extraBytes = length % sizeof(word_int);


	pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Inserting buffer of %ld bytes at address " FORMAT_WORD_POINTER "\n", length, addr);

	for(curAddr = 0; curAddr < length - extraBytes; curAddr += sizeof(word_int)) {
		memcpy((void*)(&data), (void*)(buffer+curAddr), sizeof(word_int));
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Inserting word [" FORMAT_WORD_HEX "] at address " FORMAT_WORD_POINTER "\n",
				ARG_FORMAT_WORD(data), curAddr+addr);
		_ptrace(PTRACE_POKETEXT, child, (void*)(curAddr+addr), (void*)data);
	}
	if(extraBytes > 0) {
		data = 0;
		memcpy((void*)(&data), (void*)(buffer+curAddr), extraBytes);
		extraBytes <<= 3;
		// This is not needed, stupid endianness
		// data >>= (64 - extraBytes);
		word_int mask = ((1L << (extraBytes))-1);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Inserting %d extra bytes (word [" FORMAT_WORD_HEX "] at address " FORMAT_WORD_POINTER ")\n", extraBytes >> 3,
				ARG_FORMAT_WORD(data), curAddr+addr);
		data &= mask;
		word_int destData = _ptrace(PTRACE_PEEKTEXT, child, (void*)(curAddr+addr), NULL);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Replacing [" FORMAT_WORD_HEX "] by ", ARG_FORMAT_WORD(destData));
		destData &= ~(mask);
		destData |= data;
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "[" FORMAT_WORD_HEX "]\n", ARG_FORMAT_WORD(destData));
		_ptrace(PTRACE_POKETEXT, child, (void*)(curAddr+addr), (void*)destData);
	}
}

word_int trace_mmap(pid_t child, word_int addr, size_t length, int prot)
{
	int i;
	// Save user registers
	REG_STRUCT regs;
	_ptrace(PTRACE_GETREGS, child, NULL, &regs);
	REG_STRUCT saved_regs;
	_ptrace(PTRACE_GETREGS, child, NULL, &saved_regs);

#if __WORDSIZE == 64
	if(ptrace_bits == 32) {
#endif
		/*
	 	   Linux syscall on 32-bits (int 0x80 = CD 80) number __NR_mmap (90)
	 	   eax = __NR_mmap
		   ebx = arg1 => addr
		   ecx = arg2 => length
		   edx = arg3 => prot
		   esi = arg4 => flags = MAP_ANONYMOUS | MAP_PRIVATE
		   edi = arg5 => fd = -1
		   ebp = arg6 => offset = 0
		   return rax = return value = addr
		 */
		regs.REG_AX = 192; // __NR_mmap2 32-bits
		regs.REG_BX = addr;
		regs.REG_CX = length;
		regs.REG_DX = prot;
		regs.REG_SI = MAP_ANONYMOUS | MAP_PRIVATE;
		regs.REG_DI = -1;
		regs.REG_BP = 0;
#if __WORDSIZE == 64
	}
	else {
		/*
		   Linux syscall on 64-bits (syscall - 0f 05) number __NR_mmap (9)
		   rax = __NR_mmap
		   rdi = arg1 => addr
		   rsi = arg2 => length
		   rdx = arg3 => prot
		   r10 = arg4 => flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT
		   r8 = arg5 => fd = -1
		   r9 = arg6 => offset = 0
		   return rax = return value = addr
		 */
		regs.REG_AX = 9; // __NR_mmap 64-bits
		regs.REG_DI = addr;
		regs.REG_SI = length;
		regs.REG_DX = prot;
		regs.REG_R10 = MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT;
		regs.REG_R8 = -1;
		regs.REG_R9 = 0;
	}
#endif
	// Pushing new user registers
	_ptrace(PTRACE_SETREGS, child, NULL, &regs);

	// Inserting syscall (0f 05) / int 0x80 (cd 80)
	uint8_t opcode[] = { 0x0f, 0x05, 0xcd, 0x80 };
	int off = ptrace_bits == 32 ? 2 : 0;
	int size = 2;
	for(i = 0; i < size; i++)
		opcode[i+off] = trace_replace(child, saved_regs.REG_IP+i, opcode[i+off]);

	// Two step execution with PTRACE_SYSCALL to return
	_ptrace(PTRACE_SYSCALL, child, NULL, NULL);
	waitpid(child, NULL, 0); // The syscall was called
	_ptrace(PTRACE_SYSCALL, child, NULL, NULL);
	waitpid(child, NULL, 0); // The syscall has returned

	// Getting back the result from the syscall
	_ptrace(PTRACE_GETREGS, child, NULL, &regs);

	// Restoring registers
	_ptrace(PTRACE_SETREGS, child, NULL, &saved_regs);

	// Inserting back the original opcode
	for(i = 0; i < size; i++)
		opcode[i+off] = trace_replace(child, saved_regs.REG_IP+i, opcode[i+off]);

	// Returning the result of the syscall
	return regs.REG_AX;
}

char *trace_get_zstring(pid_t child, word_int addr)
{
	int size = 1024;
	unsigned int index = 0;
	if(addr == 0) return NULL;

	uint8_t *buf = (uint8_t*)malloc(size);
	do {
		if(index >= size) {
			size += 1024;
			buf = (uint8_t*)realloc(buf, size);
		}
		trace_read(child, addr+index, buf+index, 1);
		index++;
	} while(buf[index-1]);
	return (char*)buf;
}

int syscall_match_arg(pid_t child, int type, va_list* args, word_int reg)
{
	word_int value;
	char *ptr;
	char **ptr2;
	char *buf;
	int r;

	switch(type) {
	case SYSCALL_ARGTYPE_INT:
		value = va_arg(*args, word_int);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, " %ld == %ld?", reg, value);
		return (value == reg) ? 1 : 0;

	case SYSCALL_ARGTYPE_ZSTRING:
		ptr = va_arg(*args, char*);
		if(ptr == NULL) {
			pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, " NULL == %p?", (void*)reg);
			return reg == 0 ? 1 : 0;
		}
		buf = trace_get_zstring(child, reg);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, " \"%s\" (%p) == \"%s\"?", buf, (void*)reg, ptr);
		r = strcmp(ptr, buf);

		free(buf);
		return r == 0 ? 1 : 0;

	case SYSCALL_ARGTYPE_ZSTRING_RETURN:
		buf = trace_get_zstring(child, reg);
		ptr2 = va_arg(*args, char**);
		if(ptr2 == NULL) {
			*ptr2 = NULL;
			pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, " got NULL for getting zstring...");
		}
		else {
			*ptr2 = trace_get_zstring(child, reg);
			pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, " returning string %s...", *ptr2);
		}
		return 1;
	case SYSCALL_ARGTYPE_IGNORE:
	default:
		pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, " Ignoring %lx!", reg);
		return 1;
	}
}

int syscall_match_args(pid_t child, int syscall, va_list* args, REG_STRUCT regs)
{
	int type;
#define _sma_TEST_ARG(child, type, args, arg) \
		do { \
			type = va_arg(*args, int); \
			if(type == SYSCALL_ARGTYPE_END) \
				return 1; \
			if(syscall_match_arg(child, type, args, arg) == 0) \
				return 0; \
		} while(0)

	// 64bits args: rdi,rsi,rdx,r10,r8,r9
	// 32bits args: rbx,rcx,rdx,rsi,rdi,rbp
	if(ptrace_bits == 32) {
		if(syscall == TRACE_SYSCALL(mmap)) {
			uint32_t buf[6];
			trace_read(child, regs.REG_BX, (uint8_t*)buf, 24);
			pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "\tFound old_mmap call and reading memory from ebx:\n");
			pptrace_dump_buffer(PPTRACE_DEBUG_LEVEL_ALL, (uint8_t*)buf, 24);
			pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "\nArgument check: ");
			_sma_TEST_ARG(child, type, args, buf[0]);
			_sma_TEST_ARG(child, type, args, buf[1]);
			_sma_TEST_ARG(child, type, args, buf[2]);
			_sma_TEST_ARG(child, type, args, buf[3]);
			_sma_TEST_ARG(child, type, args, buf[4]);
			_sma_TEST_ARG(child, type, args, buf[5]);
		}
		else {
			_sma_TEST_ARG(child, type, args, regs.REG_BX);
			_sma_TEST_ARG(child, type, args, regs.REG_CX);
			_sma_TEST_ARG(child, type, args, regs.REG_DX);
			_sma_TEST_ARG(child, type, args, regs.REG_SI);
			_sma_TEST_ARG(child, type, args, regs.REG_DI);
			_sma_TEST_ARG(child, type, args, regs.REG_BP);
		}
	}
	else {
		_sma_TEST_ARG(child, type, args, regs.REG_DI);
		_sma_TEST_ARG(child, type, args, regs.REG_SI);
		_sma_TEST_ARG(child, type, args, regs.REG_DX);
		_sma_TEST_ARG(child, type, args, regs.REG_R10);
		_sma_TEST_ARG(child, type, args, regs.REG_R8);
		_sma_TEST_ARG(child, type, args, regs.REG_R9);
	}
	return 1;
}

#if __WORDSIZE == 32
#define SYSCALLS_FORMAT "%u,%u"
#define SYSCALLS_PRINT_ARGS(syscalls) (syscalls & 0xffff), ((syscalls >> 16) & 0xffff)
#define TEST_SYSCALL(syscalls, syscall) (((syscall != 0) || (syscall == syscalls)) && (((syscalls & 0xffff) == syscall) || (((syscalls >> 16) & 0xffff) == syscall)))
#else
#define SYSCALLS_FORMAT "%u,%u,%u,%u"
#define SYSCALLS_PRINT_ARGS(syscalls) (syscalls & 0xffff), ((syscalls >> 16) & 0xffff), ((syscalls >> 32) & 0xffff), ((syscalls >> 48) & 0xffff)
#define TEST_SYSCALL(syscalls, syscall) (((syscall != 0) || (syscall == syscalls)) && (((syscalls & 0xffff) == syscall) || (((syscalls >> 16) & 0xffff) == syscall) || (((syscalls >> 32) & 0xffff) == syscall) || (((syscalls >> 48) & 0xffff) == syscall)))
#endif
int trace_wait_syscall(pid_t child, word_int* retval, unsigned long syscall, ...)
{
	// Wait for syscall and return the return value of the first encountered
	int status;
	REG_STRUCT regs;
	int in_syscall = 0;
	int in_execve = 0;
	va_list args;

	pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Looking for syscalls "SYSCALLS_FORMAT"\n", SYSCALLS_PRINT_ARGS(syscall));
	for (status = trace_syscall(child); !WIFEXITED(status) && !WIFSIGNALED(status); status = trace_syscall(child)) {
		if(WIFSTOPPED(status)) {
			int signal = WSTOPSIG(status);
			if(signal == SIGTRAP) {
				_ptrace(PTRACE_GETREGS, child, NULL, &regs);
				if(!in_syscall) {
					in_syscall = regs.REG_ORIG_AX;
					pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Entering syscall %d\n", in_syscall);
					if(in_syscall == TRACE_SYSCALL(execve)) in_execve = 1; // execve generate 3 sigtraps: one at call, one at program loading and one at syscall return
					if (TEST_SYSCALL(syscall, in_syscall)) {
						pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Found syscall entry %d with registers "REG_FORMAT"\n", in_syscall, REG_FORMAT_VALUES(regs));
						// We are in the good syscall but have we the good arguments?
						va_start(args, syscall);
						pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Checking arguments: ");
						if(!syscall_match_args(child, in_syscall, &args, regs)) {
							in_syscall = -in_syscall; // Arguments doesn't match
							pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, " failed!\n");
						}
						else {
							pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, " ok\n");
						}
						va_end(args);
					}
				}
				else if(in_execve) {
					pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "execve has stopped after loading the new program\n");
					in_execve = 0;
				}
				else {
					// Exiting a syscall
					pptrace_debug(PPTRACE_DEBUG_LEVEL_ALL, "Exiting syscall %d\n", in_syscall);
					if(TEST_SYSCALL(syscall, in_syscall)) {
						pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Found syscall %d with registers "REG_FORMAT"\n", in_syscall, REG_FORMAT_VALUES(regs));
						if(retval != NULL)
							*retval = regs.REG_AX;
						return 0;
					}
					in_syscall = 0;
				}
			}
			else if (signal == SIGSEGV || signal == SIGKILL || signal == SIGILL) {
				// Signal that terminates the process
				pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Syscalls "SYSCALLS_FORMAT" not found\n", SYSCALLS_PRINT_ARGS(syscall));
				return -1;
			}
			else pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Stopped with unknown signal %d\n", signal);
		} else pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Waitpid returned with the process unstopped\n");
	}
	pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Syscalls "SYSCALLS_FORMAT" not found\n", SYSCALLS_PRINT_ARGS(syscall));
	return -1;
}

void trace_get_regs(pid_t child, REG_STRUCT* regs)
{
	_ptrace(PTRACE_GETREGS, child, NULL, regs);
}

void trace_set_regs(pid_t child, REG_STRUCT* regs)
{
	_ptrace(PTRACE_SETREGS, child, NULL, regs);
}

void trace_detach(pid_t child)
{
	_ptrace(PTRACE_DETACH, child, NULL, NULL);
}


void trace_wait(pid_t child)
{
	int status = 0;
	do {
		waitpid(child, &status, 0);
	} while(!WIFEXITED(status) && !WIFSIGNALED(status));
}
