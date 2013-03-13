/*
 * pptrace.c
 *
 * Public interface of the library
 *
 *  Created on: 2 Aug. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <opcodes.h>
#include <tracing.h>
#include <isize.h>
#include <memory.h>
#include <binary.h>
#include <errors.h>
#include <hijack.h>
#include <pptrace.h>

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>


// To store informations about hijack to install
typedef struct __pptrace_internal_hijack {
	char* function;        // Function to hijack
	char* orig_symbol;     // Name of the symbol to store the intercepted address
	char* replace_symbol;  // Name of the interceptor symbol
	zzt_symbol* funSym;    // Symbol of the function
	zzt_symbol* origSym;   // Symbol to store the intercepted address
	zzt_symbol* replSym;   // Interceptor symbol
} pptrace_internal_hijack;

// To store informations about libraries to load
typedef struct __pptrace_internal_library {
	char* library;                     // The library name
	pptrace_internal_hijack** hijacks; // The hijacks to install

	zzt_word baseaddr;

	// Double-linked list
	struct __pptrace_internal_library* next;
	struct __pptrace_internal_library* prev;
} pptrace_internal_library;

// And finally the binary structure
typedef struct __pptrace_internal_binary {
	char *name;    // Name of the binary
	void *binary;  // Binary structure

	char **debugger; // Argument list to debugger an argument matching "{name}" will be replaced by the binary name and arguments matching "{pid}" will be replaced by the program pid
	// List of libraries
	pptrace_internal_library* first;
	pptrace_internal_library* last;
} pptrace_internal_binary;

#define ALLOC(type) (type*)malloc(sizeof(type));

// Look out for a binary in the path
char* get_program_path(char *name)
{
	struct stat buf;
	if(stat(name, &buf) == 0) { // The address is relative
		return strdup(name);
	}

	// Looking in the path
	char *path = getenv("PATH");
	if(path == NULL) return NULL;
	path = strdup(path); // copying to avoid corrupting the environment variable
	if(path == NULL) return NULL;

	char *ifs = getenv("IFS");
	if(ifs == NULL) ifs = ":";

	// for each path
	char *p;
	char rpath[1024];
	for(p = strtok(path, ifs); p != NULL; p = strtok(NULL, ifs)) {
		// Add the path component
		if(p[strlen(p)-1] != '/')
			snprintf(rpath, 1024, "%s/%s", p, name);
		else
			snprintf(rpath, 1024, "%s%s", p, name);
		// Test if it exists
		if(stat(rpath, &buf) == 0) {
			free(path);
			return strdup(rpath);
		}
	}
	free(path);
	return NULL;
}

void* pptrace_prepare_binary(char *binary)
{
	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Loading binary %s... ", binary);

	pptrace_clear_error();
	pptrace_internal_binary* bin = ALLOC(pptrace_internal_binary);
	if(!bin) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "failed!\n");
		pptrace_error("Allocation failed");
		return NULL;
	}
	bin->debugger = NULL;
	bin->first = bin->last = NULL;
	bin->name = get_program_path(binary);
	if(!(bin->name)) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "failed!\n");
		free(bin);
		pptrace_error("Allocation failed");
		return NULL;
	}
	bin->binary = open_binary(bin->name);
	if(!(bin->binary)) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "failed!\n");
		pptrace_error("Cannot open binary %s", binary);
		free(bin->name);
		free(bin);
		return NULL;
	}

	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "ok\n");
	return (void*)bin;
}

void  pptrace_add_debugger(void *bin, char **argv)
{
	pptrace_internal_binary* ibin = bin;
	ibin->debugger = argv;
	if(argv == NULL)
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Installing debugger %s\n");
	else
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Uninstalling debugger\n", argv[0]);
}


void pptrace_free_hijack(pptrace_internal_hijack* hijack)
{
	if(!hijack) return;

	if(hijack->origSym) free_symbol(hijack->origSym);
	if(hijack->replSym) free_symbol(hijack->replSym);
	if(hijack->funSym) free_symbol(hijack->funSym);

	if(hijack->orig_symbol) free(hijack->orig_symbol);
	if(hijack->replace_symbol) free(hijack->replace_symbol);
	if(hijack->function) free(hijack->function);

	free(hijack);
}

pptrace_internal_hijack* pptrace_get_hijack(pptrace_internal_binary *bin, char *library, void *lib, char *params)
{
	char *params_copy;
	char *function;
	char *interceptee;
	char *interceptor;

	// Parsing of the result of __pptrace_hijack_list
	params_copy = strdup(params);
	function = strtok(params_copy, " ");
	interceptee = strtok(NULL, " ");
	interceptor = strtok(NULL, " ");

	if(function == NULL || interceptee == NULL || interceptor == NULL) {
		pptrace_error("Wrong order to intercept: %s\n", params);
		free(params_copy);
		return NULL;
	}
	pptrace_internal_hijack *hijack = ALLOC(pptrace_internal_hijack);

	pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "Got order to intercept %s with %s using %s\n", function, interceptor, interceptee);
	if(!hijack) {
		pptrace_error("Allocation failed");
		free(params_copy);
		return NULL;
	}

	hijack->orig_symbol = hijack->replace_symbol = hijack->function = NULL;
	hijack->origSym = hijack->replSym = hijack->funSym = NULL;

	hijack->orig_symbol = strdup(interceptee);
	if(!hijack->orig_symbol) {
		pptrace_free_hijack(hijack);
		pptrace_error("Allocation failed");
		free(params_copy);
		return NULL;
	}
	hijack->replace_symbol = strdup(interceptor);
	if(!hijack->replace_symbol) {
		pptrace_free_hijack(hijack);
		pptrace_error("Allocation failed");
		free(params_copy);
		return NULL;
	}
	hijack->function = strdup(function);
	if(!hijack->function) {
		pptrace_free_hijack(hijack);
		pptrace_error("Allocation failed");
		free(params_copy);
		return NULL;
	}
	free(params_copy);

	hijack->funSym = get_symbol(bin->binary, hijack->function);
	if(!hijack->funSym) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Failed to find symbol %s in binary %s, ignoring!\n", hijack->function, library);
		pptrace_free_hijack(hijack);
		return NULL;
	}

	hijack->origSym = get_symbol(lib, hijack->orig_symbol);
	if(!hijack->origSym) {
		pptrace_error("Failed to find symbol %s in binary %s", hijack->orig_symbol, library);
		pptrace_free_hijack(hijack);
		return NULL;
	}

	hijack->replSym = get_symbol(lib, hijack->replace_symbol);
	if(!hijack->replSym) {
		pptrace_error("Failed to find symbol %s in binary %s", hijack->replace_symbol, library);
		pptrace_free_hijack(hijack);
		return NULL;
	}

	return hijack;
}

void pptrace_free_hijacks(pptrace_internal_hijack** hijacks)
{
	int i;
	if(!hijacks) return;
	for(i = 0; hijacks[i] != NULL; i++) pptrace_free_hijack(hijacks[i]);
	free(hijacks);
}

pptrace_internal_hijack** pptrace_get_hijacks(pptrace_internal_binary *bin, char *library, void *lib, char **params)
{
	int s, i, j;
	for(s = 0; params[s] != NULL; s++);

	pptrace_internal_hijack** hijacks = (pptrace_internal_hijack**)malloc(sizeof(pptrace_internal_hijack*)*(s+1));

	if(!hijacks) {
		pptrace_error("Allocation failed");
		return NULL;
	}

	for(i = 0; i < s+1; i++) hijacks[i] = NULL;

	for(i = j = 0; j < s; i++, j++) {
		hijacks[i] = pptrace_get_hijack(bin, library, lib, params[j]);
		if(!hijacks[i]) i--; // ignoring this line
	}
	return hijacks;
}

pptrace_internal_hijack** pptrace_load_hijacks(pptrace_internal_binary *bin, char *library, void *lib)
{
	void *bin_bin = open_binary(library);
	char** params = read_zstring_array(bin_bin, PPTRACE_HIJACK_FUNCTION_NAME);
	close_binary(bin_bin);

	if(!params) {
		pptrace_error("%s library does not contain symbol %s, probably wrong library format", library, PPTRACE_HIJACK_FUNCTION_NAME);
		return NULL;
	}

	pptrace_internal_hijack **result = pptrace_get_hijacks(bin, library, lib, params);
	free_zstring_array(params);
	return result;
}

void pptrace_free_library(pptrace_internal_library *lib)
{
	if(!lib) return;

	if(lib->library) {
		free(lib->library);
	}
	if(lib->hijacks) {
		pptrace_free_hijacks(lib->hijacks);
	}
	free(lib);
}

void pptrace_insert_library(pptrace_internal_binary* binary, pptrace_internal_library* lib)
{
	lib->next = NULL;
	lib->prev = binary->last;
	if(binary->last) {
		binary->last->next = lib;
		binary->last = lib;
	}
	else {
		binary->first = binary->last = lib;
	}
}

void pptrace_remove_last_library(pptrace_internal_binary* binary)
{
	if(binary->last) {
		pptrace_internal_library* lib = binary->last;
		binary->last = lib->prev;
		if(lib->prev) {
			lib->prev->next = NULL;
		}
		if(binary->first == lib) binary->first = NULL;
	}
}

int   pptrace_load_module(void *bin, char *library)
{
	int r = pptrace_add_preload(bin, library);
	if(r < 0)
		return r;

	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Fetching hijacks of library %s\n", library);
	pptrace_internal_binary* binary = (pptrace_internal_binary*)bin;
	pptrace_internal_library* lib = binary->last;
	void *libr = open_binary(library);
	if(!binary) {
		pptrace_remove_last_library(binary);
		pptrace_free_library(lib);
		pptrace_error("Failed to open library %s", library);
		return -1;
	}
	lib->hijacks = pptrace_load_hijacks(binary, library, libr);
	if(!lib->hijacks) {
		pptrace_remove_last_library(binary);
		pptrace_free_library(lib);
		return -1;
	}
	close_binary(libr);
	lib->baseaddr = 0;

	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Hijacks of library %s fetched\n", library);
	return 0;
}

int   pptrace_add_preload(void *bin, char *library)
{
	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Loading library %s... ", library);
	pptrace_clear_error();
	if(!bin || !library) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "failed!\n");
		pptrace_error("Invalid argument");
		return -1;
	}
	pptrace_internal_binary* binary = (pptrace_internal_binary*)bin;
	pptrace_internal_library* lib = ALLOC(pptrace_internal_library);

	if(!lib) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "failed!\n");
		pptrace_error("Allocation failed");
		return -1;
	}

	lib->library = strdup(library);
	lib->baseaddr = -1;
	if(!(lib->library)) {
		free(lib);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "failed!\n");
		pptrace_error("Allocation failed");
		return -1;
	}

	lib->hijacks = NULL;
	pptrace_insert_library(binary, lib);
	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "ok\n");
	return 0;
}

char* pptrace_create_preload(pptrace_internal_library *first)
{
	const static char* ldpreload = "LD_PRELOAD=";
	char *result = (char*)malloc(1024);
	int size = 1024;
	int nsize = strlen(ldpreload);
	strcpy(result, ldpreload);
	while(first != NULL) {
		while(size-nsize < strlen(first->library)+2) {
			size += 1024;
			result = (char*)realloc(result, size);
		}
		strcpy(result+nsize, first->library);
		nsize += strlen(first->library);
		strcpy(result+nsize, ":");
		nsize++;
		first = first->next;
	}
	char *envpreload = getenv("LD_PRELOAD");
	if(envpreload != NULL) {
		while(size-nsize < strlen(envpreload)+1) {
			size += 1024;
			result = (char*)realloc(result, size);
		}
		strcpy(result+nsize, envpreload);

	} else if(nsize > 0) {
		nsize--; // remove the trailing ':'
		result[nsize] = 0;
	}
	return result;
}

pid_t pptrace_launch_preload(char *path, char **argv, char **envp, pptrace_internal_library *first, int debug) {
	// Adding LD_PRELOAD=libraries in envp
	int i, size = 0;
	while(envp[size] != NULL)
		size++;

	char **envp2 = (char**)malloc(sizeof(char*)*(size+2));
    for(i = 0; i < size; i++) {
        envp2[i] = envp[i];
	}
	envp2[size] = pptrace_create_preload(first);
	envp2[size+1] = NULL;
	
	pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "\nLD_PRELOAD is %s\n", envp2[size]);
	
	pid_t child = trace_run(path, argv, envp2, debug);
	free(envp2[size]);
	free(envp2);
	return child;
}

void pptrace_free_binary(pptrace_internal_binary* binary)
{
	pptrace_internal_library *lib;
	if(!binary) return;

	if(binary->binary) {
		close_binary(binary->binary);
	}
	while(binary->first) {
		lib = binary->first;
		binary->first = lib->next;
		pptrace_free_library(lib);
	}
	binary->last = NULL;
	if(binary->name) {
		free(binary->name);
	}
	free(binary);
}

int pptrace_count_unmatched_libraries(pptrace_internal_binary* binary, int max)
{
	int count = 0;
	pptrace_internal_library *result = binary->first;
	while(result != NULL && count < max) {
		if(result->baseaddr == 0) count++;
		result = result->next;
	}
	return count;
}

pptrace_internal_library* pptrace_search_library(pptrace_internal_binary* binary, char *libname)
{
	pptrace_internal_library *result = binary->first;
	int libnamelen = strlen(libname);
	while(result != NULL) {
		int len = libnamelen - strlen(result->library);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_DEBUG, "Comparing %s and %s\n", result->library, libname+len);
		if(len >= 0 && strcmp(result->library, libname+len) == 0) // the searched library ends with the short name of the lib
			return result;
		result = result->next;
	}
	return NULL;
}

void pptrace_apply_library_baseaddr(pptrace_internal_library *lib, zzt_word baseaddr)
{
	int i;
	lib->baseaddr = baseaddr;
	if(lib->hijacks) {
		for(i = 0; lib->hijacks[i]; i++) {
			lib->hijacks[i]->origSym->section_addr += baseaddr;
			lib->hijacks[i]->replSym->section_addr += baseaddr;
		}
	}
}

int pptrace_wait_library(pid_t child, pptrace_internal_binary* binary)
{
	char *library;
	// Wait for dlopen(argv[1])
	pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "Waiting for open(*, O_RDONLY) (syscall %d)... ", TRACE_SYSCALL(open));
	word_int fd;
	int r = trace_wait_syscall(child, &fd, TRACE_SYSCALL(open), SYSCALL_ARGTYPE_ZSTRING_RETURN, &library, SYSCALL_ARGTYPE_IGNORE, SYSCALL_ARGTYPE_END);
	if (r < 0 || library == NULL) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "failed!\n");
		return -1;
	}
	pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "ok (fd = %d, library = %s)\n", fd, library);

	if((int)fd < 0) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "open() failed ==> ignoring\n");
		return 0;
	}

	pptrace_internal_library* lib = pptrace_search_library(binary, library);
	free(library);
	if(!lib || lib->baseaddr) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "Not an awaited library, ignoring\n");
		return 0;
	}

	pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "Waiting for mmap(*, *, PROT_READ|PROT_EXEC, *, %d, *) (syscall %d)... ", fd, MMAP_SYSCALLS);
	word_int baseaddr = 0;
	r = trace_wait_syscall(child, &baseaddr, MMAP_SYSCALLS,
			SYSCALL_ARGTYPE_IGNORE,
			SYSCALL_ARGTYPE_IGNORE,
			SYSCALL_ARGTYPE_INT, (word_int)(PROT_READ|PROT_EXEC),
			SYSCALL_ARGTYPE_IGNORE,
			SYSCALL_ARGTYPE_INT, (word_int)fd,
			SYSCALL_ARGTYPE_IGNORE,
			SYSCALL_ARGTYPE_END);
	if (baseaddr == 0 || r < 0) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "failed!\n");
		return -1;
	}
	pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "ok (addr = %016lx)\n", baseaddr);
	pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "Waiting for close(%d) (syscall %d)... ", fd, TRACE_SYSCALL(close));
	if(trace_wait_syscall(child, NULL, TRACE_SYSCALL(close), SYSCALL_ARGTYPE_INT, fd, SYSCALL_ARGTYPE_END) < 0) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "failed!\n");
		return -1;
	}
	pptrace_debug(PPTRACE_DEBUG_LEVEL_VERBOSE, "ok (addr = %016lx)\n", baseaddr);
	pptrace_apply_library_baseaddr(lib, (zzt_word)baseaddr);
	return 0;
}

int pptrace_wait_libraries(pid_t child, pptrace_internal_binary* binary)
{
	while(pptrace_count_unmatched_libraries(binary, 1)) {
		if(pptrace_wait_library(child, binary)) {
			return -1;
		}
	}
	return 0;
}

int pptrace_install_hijacks(pid_t child, pptrace_internal_binary* binary)
{
	int i;
	pptrace_internal_library* lib;
	for(lib = binary->first; lib != NULL; lib = lib->next) {
		if(lib->hijacks) {
			for(i = 0; lib->hijacks[i] != NULL; i++) {
				if(lib->hijacks[i]->funSym != NULL && lib->hijacks[i]->funSym->symbol_offset) {
					// Replacing symbol only when the symbol is not in a dynamic library
					if(hijack(binary->binary, child, lib->hijacks[i]->funSym, lib->hijacks[i]->origSym,
							lib->hijacks[i]->replSym) < 0) {
						pptrace_error("Failed to install hijack of symbol %s", lib->hijacks[i]->function);
						return -1;
					}
				}
			}
		}
	}
	return 0;
}

int  pptrace_run(void *bin, char **argv, char **envp)
{
	pptrace_clear_error();
	if(!bin) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "failed!\n");
		pptrace_error("Invalid argument");
		return -1;
	}
	pptrace_internal_binary* binary = (pptrace_internal_binary*)bin;
	trace_set_bits(get_binary_bits(binary->binary));
	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Running binary %s... ", binary->name);
	pid_t child = pptrace_launch_preload(binary->name, argv, envp, binary->first, (binary->debugger != NULL) ? 1 : 0);
	if(child <= 0) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "failed!\n");
		pptrace_error("Failed to run binary %s", binary->name);
		pptrace_free_binary(binary);
		return -1;
	}
	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "ok (pid = %d)\n", child);

	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Getting libraries' addresses\n");
	if(pptrace_wait_libraries(child, binary)) {
		pptrace_error("Failed to wait for open of the library, exiting after detaching child process...");
		pptrace_free_binary(binary);
		return -1;
	}

	pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Installing hijacks\n");
	if(pptrace_install_hijacks(child, binary)) {
		trace_detach(child);
		pptrace_error("Failed to install hijacks, exiting after detaching child process...");
		pptrace_free_binary(binary);
		return -1;
	}

	if(binary->debugger != NULL) {
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Giving the control to the debugger %s\n", binary->debugger[0]);
		char pid[10];
		int i;
		snprintf(pid, 10, "%d", child);
		pid[9]=0;
		for(i = 0; binary->debugger[i] != NULL; i++) {
			if(strcmp(binary->debugger[i], "{pid}") == 0) {
				binary->debugger[i] = pid;
			} else if(strcmp(binary->debugger[i], "{name}") == 0) {
				binary->debugger[i] = binary->name;
			}
		}
		char *debugger = get_program_path(binary->debugger[0]);
		if(debugger == NULL) {
			pptrace_fubar("debugger %s was not found!", binary->debugger[0]);
		}
		trace_detach(child);
		execve(debugger, binary->debugger, envp);
		pptrace_fubar("failed to launch debugger!");
	} else {
		pptrace_free_binary(binary);
		pptrace_debug(PPTRACE_DEBUG_LEVEL_INFO, "Detaching and waiting the end of the process\n");
		trace_detach(child);
		trace_wait(child);
	}
	return 0;
}
