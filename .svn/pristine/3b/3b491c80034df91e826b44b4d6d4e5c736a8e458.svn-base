/*
 * wait_open.c - test waiting for open and mmap in both 32 and 64 bits
 *
 *  Created on: 5 Aug. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <string.h>
#include <binary.h>
#include <tracing.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

char **new_envp(char **envp, char *ldpreload)
{
	int i;
	for(i = 0; envp[i]; i++);
	char **env = (char**)malloc(sizeof(char*) * (i+1));
	env[0] = (char*)malloc(15+strlen(ldpreload));
	snprintf(env[0], 15+strlen(ldpreload), "LD_PRELOAD=%s", ldpreload);
	for(i = 0; envp[i]; i++) env[i+1] = envp[i];
	env[i+1] = NULL;
	return env;
}

int test(int debug, char *prog, char *lib, char **envp)
{
	char *argv[] = { NULL, "-s", NULL };
	argv[0] = prog;
	char **env = new_envp(envp, lib);

	void *bin = open_binary(prog);
	if(bin == NULL) {
		fprintf(stderr, "Failed to open binary %s!\n", prog);
		return -__LINE__;
	}
	trace_set_bits(get_binary_bits(bin));
	close_binary(bin);

	pid_t child = trace_run(prog, argv, env, 1);
	//memory leak here to avoid double free on hydra
	//free(env[0]);
	//free(env);
	if(child <= 0) {
		fprintf(stderr, "Failed to trace process %s!\n", prog);
		return -__LINE__;
	}

	//open("/etc/ld.so.cache", *)
	word_int fd;
	if(trace_wait_syscall(child, &fd, TRACE_SYSCALL(open), SYSCALL_ARGTYPE_ZSTRING, lib, SYSCALL_ARGTYPE_IGNORE, SYSCALL_ARGTYPE_END) < 0) {
		fprintf(stderr, "Failed to wait for open(\"%s\", *) for process %s (pid = %d)!\n", lib, prog, child);
		trace_detach(child);
		return -__LINE__;
	}
	if(debug)
		printf("open(\"%s\", *) = "WORD_DEC_FORMAT"\n", lib, fd);
	if((int)fd < 3) {
		fprintf(stderr, "open(\"%s\", *) returned a negative value or a reserved file descriptor ("WORD_DEC_FORMAT" - "WORD_HEX_FORMAT") for process %s (pid = %d)!\n", lib, fd, fd, prog, child);
		return -__LINE__;
	}

	//mmap(NULL, 22967, PROT_READ, MAP_PRIVATE,  3, 0) = 0x7f4b50756000
	//mmap(NULL, *,             *,           *, fd, 0)
	word_int addr;
	if(trace_wait_syscall(child, &addr, MMAP_SYSCALLS,
			SYSCALL_ARGTYPE_IGNORE,
			SYSCALL_ARGTYPE_IGNORE,
			SYSCALL_ARGTYPE_INT, (word_int)(PROT_READ|PROT_EXEC),
			SYSCALL_ARGTYPE_IGNORE,
			SYSCALL_ARGTYPE_INT, (word_int)fd,
			SYSCALL_ARGTYPE_IGNORE,
			SYSCALL_ARGTYPE_END) < 0) {
		fprintf(stderr, "Failed to wait for mmap(*, *, PROT_READ|PROT_EXEC, *, "WORD_DEC_FORMAT", *) for process %s (pid = %d)!\n", fd, prog, child);
			trace_detach(child);
			return -__LINE__;
	}

	if(debug)
		printf("mmap(*, *, PROT_READ|PROT_EXEC, *, "WORD_DEC_FORMAT", *) = "WORD_HEX_FORMAT"\n", fd, addr);
	if((void*)addr == MAP_FAILED) {
		fprintf(stderr, "mmap(*, *, PROT_READ|PROT_EXEC, *, "WORD_DEC_FORMAT", *) returned MAP_FAILED for process %s (pid = %d)!\n", fd, prog, child);
		return -__LINE__;
	}
	trace_detach(child);
	trace_wait(child);
	return EXIT_SUCCESS;
}

int main(int argc, char **argv, char **envp)
{
	int debug = 0;
	int i;
	for(i = 1; argv[i] != NULL; i++) {
		if(strcasecmp(argv[i], "-d") == 0) {
			debug = 1;
		}
		else if(strstr(argv[i], ".so") == NULL){
			char lib[1024];
			strcpy(lib, argv[i]);
			strcat(lib, ".so");
			int r = test(debug, argv[i], lib, envp);
			if(r) {
				return r;
			}
		}
	}
	return EXIT_SUCCESS;
}
