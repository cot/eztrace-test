/*
 * bfd.c
 *
 * Parsing of binaries using the BFD library. See binary.h for the interfaces
 * This file needs the -lbfd flag when linking
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <bfd.h>
#include <stdarg.h>

// Internals
int _initialized = 0;
void _zzt_bin_init() {
	if(_initialized == 0) {
		bfd_init();
	}
}

zzt_word zzt_asymbol_size(asymbol *sym)
{
	if(bfd_asymbol_flavour(sym) == bfd_target_elf_flavour) {
		// ELF case hack by reverse engineering libbfd...
		bfd_vma* ielf = (bfd_vma*)(sym+1); // The elf_internal_sym structure can be found right after the bfd struct
		return *(ielf+1); // and second word in that struct is the size of the symbol
	}
	else {
		// We don't know
		return 0;
	}
}

void* apply_on_symbol(void *bin, char *symbol, void* (*apply)(bfd *abfd, asymbol *sym, va_list ap), ...)
{
	va_list ap;
	bfd* abfd = (bfd*)bin;
	long storage_needed;
	asymbol **symbol_table;
	long number_of_symbols;
	long i;

	va_start(ap, apply);
	storage_needed = bfd_get_symtab_upper_bound (abfd);

	if (storage_needed <= 0) // < 0 error, =0: not found
		return NULL;

	symbol_table = (asymbol **) malloc (storage_needed);

	number_of_symbols =
			bfd_canonicalize_symtab (abfd, symbol_table);

	if (number_of_symbols < 0)
		return NULL;

	for (i = 0; i < number_of_symbols; i++) {
		if(strcmp(symbol_table[i]->name, symbol) == 0) {
			void* result = apply(abfd, symbol_table[i], ap);
			va_end(ap);
			free(symbol_table);
			return result;
		}
	}
	va_end(ap);
	free(symbol_table);
	return NULL;
}

const char* pptrace_bin_error()
{
	if(bfd_get_error() == bfd_error_no_error) return NULL;
	return bfd_errmsg(bfd_get_error());
}

// free_symbol
void free_symbol(zzt_symbol *symbol)
{
	if(symbol) {
		if(symbol->symbol_name)
			free(symbol->symbol_name);
		symbol->symbol_name = NULL;

		if(symbol->section_name)
			free(symbol->section_name);
		symbol->section_name = NULL;

		free(symbol);
	}
}

// open_binary
void* open_binary(char *path)
{
	bfd * bfd;
	_zzt_bin_init();

	bfd = bfd_openr(path, NULL);
	if(!bfd) {
		return NULL;
	}

	if(!bfd_check_format(bfd, bfd_object)) {
		bfd_close(bfd);
		return NULL;
	}
	return bfd;
}

// close_binary
void close_binary(void *bin)
{
	bfd_close((bfd*)bin);
}

int get_binary_bits(void *bin)
{
	return bfd_arch_bits_per_address((bfd*)bin);
}

// get_symbol
void* symbol_from_bfd_sym(bfd *abfd, asymbol *sym, va_list ap)
{
	zzt_symbol *result;

	result = (zzt_symbol*)malloc(sizeof(zzt_symbol));
	result->symbol_name = strdup(sym->name);
	result->symbol_offset = sym->value;
	if(bfd_get_section(sym) == NULL) {
		result->section_name = "";
		result->section_addr = 0;
	} else {
		result->section_name = strdup(bfd_get_section(sym)->name);
		result->section_addr = bfd_asymbol_base(sym);
	}
	result->symbol_size = zzt_asymbol_size(sym);
	return (void*)result;
}

zzt_symbol* get_symbol(void *bin, char *symbol)
{
	return (zzt_symbol*)apply_on_symbol(bin, symbol, symbol_from_bfd_sym);
}

// read_symbol
void* read_symbol_from_bfd_sym(bfd *abfd, asymbol *sym, va_list ap)
{
	void* buffer = va_arg(ap, void*);
	ssize_t size = va_arg(ap, ssize_t);
	size_t result = zzt_asymbol_size(sym);
	if(result == 0 || result > size)
		result = size;
	if(bfd_get_section(sym) != NULL) {
		if(bfd_get_section_contents(abfd, bfd_get_section(sym), buffer, sym->value, result)) {
			return (void*)result;
		}
	}
	return (void*)0;
}

size_t read_symbol(void *bin, zzt_symbol *symbol, void *buffer, ssize_t size)
{
	return (size_t)apply_on_symbol(bin, symbol->symbol_name, read_symbol_from_bfd_sym, buffer, size);
}

// read_zstring
struct _get_section_struct {
	asection *result;
	zzt_word addr;
};

size_t read_zstring(void *bin, zzt_symbol *symbol, zzt_word addr, void *buffer, ssize_t size)
{
	asection *sect;
	uint8_t buf;
	bfd *abfd = (bfd*)bin;
    for (sect = abfd->sections; sect != NULL; sect = sect->next) {
    	if(sect->vma <= addr && addr < sect->vma + sect->size) {
    		addr -= sect->vma;
    		if(size > sect->size - addr) {
    			size = sect->size - addr;
    		}
			size_t result;

			for(result = 0; result < size - 1 && bfd_get_section_contents(abfd, sect, &buf, addr+result, 1) && buf != 0; result++) {
				((uint8_t*)buffer)[result] = buf;
			}
			((uint8_t*)buffer)[result] = 0;
			result++;
			return result;
		}
	}
	return 0;
}
