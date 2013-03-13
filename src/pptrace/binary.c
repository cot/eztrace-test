/*
 * binary.c
 *
 * Wrapper to include the good library for parsing of binary files
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <config.h>
#include <errors.h>
#include <binary.h>

#include <stdlib.h>
#include <string.h>

#ifndef __PPTRACE_BINARY_TYPE

#include "binary/bfd.c"

#else

#if __PPTRACE_BINARY_TYPE == PPTRACE_BINARY_TYPE_ELF
#include "binary/elf.c"
#else // if ! (__PPTRACE_BINARY_TYPE == PPTRACE_BINARY_TYPE_ELF)
#include "binary/bfd.c"
#endif // ! (__PPTRACE_BINARY_TYPE == PPTRACE_BINARY_TYPE_ELF)

#endif // !defined(__PTRACE_BINARY_TYPE)

char** read_zstring_array(void *bin, char *symbol)
{
	char **result;
	char *buf;
	char buf2[4096];

	zzt_symbol *sym = get_symbol(bin, symbol);
	int bytes = get_binary_bits(bin) >> 3;
	if(sym == NULL)
		return NULL;

	size_t size = sym->symbol_size;
	if(size <= 0)
		size = 1024;
	buf = malloc(size);
	if(buf == NULL)
		return NULL;

	uint32_t* val32 = (uint32_t*)buf;
	uint64_t* val64 = (uint64_t*)buf;

	size = read_symbol(bin, sym, buf, size) / bytes;

	result = (char**)malloc(size * sizeof(char*));

	int i = 0;
	for(i = 0; i < size; i++) {
		zzt_word w = (bytes == 4) ? val32[i] : val64[i];
		if(w == 0) {
			result[i] = 0;
			free(buf);
			return result;
		}
		size_t s = read_zstring(bin, sym, w, buf2, 1024);
		// printf("\t[%d] %p %s\n", i, (void*)w, buf2);
		result[i] = strdup(buf2);
	}
	free(buf);
	return result;
}

void free_zstring_array(char **strarray)
{
	int i = 0;
	while(strarray[i] != NULL) {
		free(strarray[i]);
		i++;
	}
	free(strarray);
}
