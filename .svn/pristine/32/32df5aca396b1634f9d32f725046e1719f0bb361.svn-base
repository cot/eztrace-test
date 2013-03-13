/*
 * elf.c
 *
 * Parsing of binaries using the GELF library. See binary.h for the interfaces
 * This file needs the -lelf flag when linking
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
#include <gelf.h>

typedef struct _zzt_my_elf_symbol_table {
	GElf_Shdr *shdr_p;
	Elf *elf;
	int fd;
	unsigned bits;
	size_t shstrndx;
	GElf_Word sh_link;
	Elf_Data *data;
} zzt_my_elf_symbol_table;

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

zzt_symbol* get_symbol(void *bin, char *symbol)
{

	char *name;
	GElf_Shdr shdr;

	zzt_my_elf_symbol_table *elf_st = (zzt_my_elf_symbol_table *)bin;

	GElf_Sym esym;
	int i;

	for (i = 0; gelf_getsym(elf_st->data, i, &esym); i++) {
		if ((esym.st_value == 0) ||
				(GELF_ST_BIND(esym.st_info) == STB_WEAK) ||
				(GELF_ST_BIND(esym.st_info) == STB_NUM) ||
				(GELF_ST_TYPE(esym.st_info) != STT_FUNC &&
				 GELF_ST_TYPE(esym.st_info) != STT_OBJECT))
			continue;
		name = elf_strptr(elf_st->elf, elf_st->sh_link, (size_t)esym.st_name);

		if(!name){
			pptrace_report_error("%s\n",elf_errmsg(elf_errno()));
			return NULL;
		}

		if(strcmp(name, symbol) == 0) {
			zzt_symbol *result = (zzt_symbol*)malloc(sizeof(zzt_symbol));
			result->symbol_name    = strdup(name);
			result->symbol_offset  = esym.st_value;
			result->symbol_size    = esym.st_size;
			Elf_Scn *scn = elf_getscn(elf_st->elf, esym.st_shndx);

			result->section_name = NULL;
			result->section_addr = 0;

			if(scn != NULL) {
				gelf_getshdr(scn, &shdr);
				result->section_name = strdup(elf_strptr(elf_st->elf, elf_st->shstrndx, shdr.sh_name));
				result->section_addr = shdr.sh_addr;
				result->symbol_offset -= result->section_addr;
				return result;
			}
		}
	}
	return NULL;
}

void* open_binary(char *path)
{
	int fd;
	Elf *e;
	Elf_Scn* section = 0;
	GElf_Shdr shdr;
	GElf_Ehdr ehdr;

	if(elf_version(EV_CURRENT) == EV_NONE) {
		pptrace_report_error("ELF library init failed: %s\n", elf_errmsg(-1));
		return NULL;
	}

	fd = open(path, O_RDONLY, 0);
	if(fd < 0) {
		pptrace_report_error("Opening %s failed\n", path);
		return NULL;
	}

	e = elf_begin(fd, ELF_C_READ, NULL);
	if(e == NULL) {
		pptrace_report_error("elf_begin() failed: %s\n", elf_errmsg(-1));
		close(fd);
		return NULL;
	}

	if(elf_kind(e) != ELF_K_ELF) {
		pptrace_report_error("Not an ELF object!\n");
		elf_end(e);
		close(fd);
		return NULL;
	}

	gelf_getehdr(e, &ehdr);

	zzt_my_elf_symbol_table *elf_st;
	while ((section = elf_nextscn(e, section)) != 0) {
		GElf_Shdr *shdr_p;
		if ((shdr_p = gelf_getshdr (section, &shdr)) != 0) {
			if (shdr_p->sh_type == SHT_SYMTAB) {
				elf_st = (zzt_my_elf_symbol_table*)malloc(sizeof(zzt_my_elf_symbol_table));
				elf_st->elf = e;
				elf_st->fd = fd;
				elf_st->sh_link = shdr_p->sh_link;
				elf_getshdrstrndx(e, &elf_st->shstrndx);

				switch(gelf_getclass(e)) {
				case ELFCLASS32: elf_st->bits = 32; break;
				case ELFCLASS64: elf_st->bits = 64; break;
				default: case ELFCLASSNONE: elf_st->bits = 0; break;
				}
				elf_st->data = 0;
				if ((elf_st->data = elf_getdata(section, elf_st->data)) == 0 || elf_st->data->d_size == 0) {
					elf_end(elf_st->elf);
					close(elf_st->fd);
					free(elf_st);
					return NULL;
				}
				return elf_st;
			}
		}
	}

	pptrace_report_error("Cannot load a symbol table!\n");
	elf_end(e);
	close(fd);

	return NULL;
}

void close_binary(void *bin)
{
	zzt_my_elf_symbol_table *elf_st = (zzt_my_elf_symbol_table *)bin;
	elf_end(elf_st->elf);
	close(elf_st->fd);
	free(elf_st);
}

int get_binary_bits(void *bin)
{
	zzt_my_elf_symbol_table *elf_st = (zzt_my_elf_symbol_table *)bin;
	return elf_st->bits;
}

const char* pptrace_bin_error() {
	return NULL;
}

Elf_Scn* find_section_by_name(zzt_my_elf_symbol_table *elf, char *name)
{
	Elf_Scn *section = NULL;
	while ((section = elf_nextscn(elf->elf, section)) != 0) {
		GElf_Shdr shdr;
		if (gelf_getshdr (section, &shdr) != 0) {
			char *sname = elf_strptr(elf->elf, elf->shstrndx, shdr.sh_name);
			if(sname != NULL) {
				if(strcmp(name, sname) == 0)
					return section;
			}
		}
	}
	return NULL;
}

Elf_Scn* find_section_by_addr(zzt_my_elf_symbol_table *elf, zzt_word* addr)
{
	Elf_Scn *section = NULL;
	while ((section = elf_nextscn(elf->elf, section)) != 0) {
		GElf_Shdr shdr;
		if (gelf_getshdr (section, &shdr) != 0) {
			if(shdr.sh_addr <= *addr && shdr.sh_size + shdr.sh_addr > *addr) {
				*addr -= shdr.sh_addr;
				return section;
			}
		}
	}
	return NULL;
}

size_t read_symbol(void *bin, zzt_symbol *symbol, void *buffer, ssize_t size)
{
	zzt_my_elf_symbol_table *elf = (zzt_my_elf_symbol_table*)bin;
	Elf_Scn *scn = find_section_by_name(elf, symbol->section_name);
	if(scn != NULL) {
		size_t result = size;
		if(result > symbol->symbol_size)
			result = symbol->symbol_size;
		Elf_Data *data = NULL;
		while(data = elf_getdata(scn, data)) {
			if(data->d_off <= symbol->symbol_offset && data->d_off + data->d_size > symbol->symbol_offset) {
				int off = symbol->symbol_offset - data->d_off;
				if(result + off > data->d_size) {
					result = data->d_size - off;
				}
				memcpy(buffer, ((uint8_t*)data->d_buf) + off, result);
				return result;
			}
		}
	}
	return 0;
}

size_t read_zstring(void *bin, zzt_symbol *symbol, zzt_word addr, void *buffer, ssize_t size)
{
	zzt_my_elf_symbol_table *elf = (zzt_my_elf_symbol_table*)bin;
	Elf_Scn *scn = find_section_by_addr(elf, &addr);
	if(scn != NULL) {
		Elf_Data *data = NULL;
		while(data = elf_getdata(scn, data)) {
			if(data->d_off <= addr && data->d_off + data->d_size > addr) {
				uint8_t *ibuf = (uint8_t*)data->d_buf;
				uint8_t *obuf = (uint8_t*)buffer;

				int off = addr - data->d_off;
				size_t result = 0;
				while(result < size - 1 && off < data->d_size && ibuf[off] != 0) {
					obuf[result] = ibuf[off];
					result++;
					off++;
				}

				obuf[result] = 0;
				result++;
				return result;
			}
		}
	}
	return 0;
}
