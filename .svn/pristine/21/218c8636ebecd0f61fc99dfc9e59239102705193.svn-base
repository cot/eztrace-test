/*
 * binary.h -- tests of binary reading functions (binary.h)
 *
 *  Created on: 4 Aug. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#include <libgen.h>
#include <binary.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

char *readline(int fd)
{
	char buf;
	size_t size = 1024;
	int index = 0;
	char *buffer = (char*)malloc(size);
	buffer[index] = 0;
	int r;
	while(0 <= (r = read(fd, &buf, 1))) {
		if(buf == '\n') {
			return buffer;
		}
		//printf("%d ", buf);
		buffer[index++] = buf;
		if(index >= size) {
			size += 1024;
			buffer = (char*)realloc(buffer, size);
		}
		buffer[index] = 0;
	}
	if(r < 0) {
		free(buffer);
		return NULL;
	}
	return buffer;
}

int compare_name(char *line, char *symbol)
{
	if(strlen(symbol)+1 >= strlen(line)) return 0;
	int i = strlen(line)-strlen(symbol)-1;
	if(!isblank(line[i])) return 0;
	return strcmp(line+i+1, symbol) == 0 ? 1 : 0;
}

char *get_nm_symbol(char *path, char *symbol)
{
	int p[2];
	if(pipe(p) == -1) return NULL;
	pid_t child = fork();
	if(child == -1) {
		close(p[1]); close(p[0]);
		return NULL;
	}
	if(child == 0) {
		close(p[0]);
		close(STDOUT_FILENO);
		dup2(p[1], STDOUT_FILENO);
		close(p[1]);
		execlp("nm", "nm", path, NULL);
		exit(-1);
	}
	else {
		close(p[1]);
		char* buffer = readline(p[0]);
		while(buffer != NULL) {
			if(compare_name(buffer, symbol)) {
				close(p[0]);
				return buffer;
			}
			free(buffer);
			buffer = readline(p[0]);
		}
		close(p[0]);
	}
	return NULL;
}

int compare_result(char *path, zzt_symbol *sym, char *symbol)
{

	if(strcmp(sym->symbol_name, symbol))
		return -__LINE__;

	char *nm = get_nm_symbol(path, symbol);

	if(!nm)
		return -__LINE__;

	zzt_word addr = strtoul(nm, NULL, 16);
	free(nm);

	if(!addr)
		return -__LINE__;

	if(addr != sym->section_addr+sym->symbol_offset)
		return -__LINE__;

	return EXIT_SUCCESS;
}

int test_read_zstring_array(void *bin, char *symbol, char **model)
{
	char **frombin = read_zstring_array(bin, symbol);
	if(frombin == NULL && model != NULL)
		return -__LINE__;
	if(model == NULL && frombin != NULL)
		return -__LINE__;

	int i = 0;
	while(frombin[i] || model[i]) {
		if(strcmp(frombin[i], model[i]) != 0)
			return -__LINE__;
		i++;
	}
	if(frombin[i])
		return -__LINE__;
	if(model[i])
		return -__LINE__;
	free_zstring_array(frombin);
	return 0;
}

int test_dump_symbol(void *bin, int bits, zzt_symbol *sym, char **model)
{
	char buf[1024];
	char buf2[1024];
	uint32_t* val32 = (uint32_t*)buf;
	uint64_t* val64 = (uint64_t*)buf;
	int i = 0;
	ssize_t size = read_symbol(bin, sym, buf, 1024);
	bits >>= 3;
	size /= bits;
	//printf("Size of symbol %s: %d\n", sym->symbol_name, (int)size);
	for(i = 0; model[i] && i < size; i++) {
		zzt_word w = (bits == 4) ? val32[i] : val64[i];
		size_t s = read_zstring(bin, sym, w, buf2, 1024);
		// printf("\t[%d] %p %s\n", i, (void*)w, buf2);
		if(strcmp(model[i], buf2) != 0)
			return -__LINE__;
	}
	if(i < size) {
		zzt_word w = (bits == 4) ? val32[i] : val64[i];
		if(w != 0)
			return -__LINE__;
	} else if(model[i]) {
		return -__LINE__;
	}
	return EXIT_SUCCESS;

}

char *sym_model[] = { "absolutely", "dummy", "heho", NULL };

int testcase(char *path, int bits, char *symbol, char *arraysym, char *nonsym)
{
	void *bin = open_binary(path);

	if(bin == NULL)
		return -__LINE__;

	if(get_binary_bits(bin) != bits)
		return -__LINE__;

	if(get_symbol(bin, nonsym) != NULL)
		return -__LINE__;

	zzt_symbol *sym = get_symbol(bin, symbol);
	if(sym == NULL)
		return -__LINE__;
	int r = compare_result(path, sym, symbol);
	free_symbol(sym);

	if(r < 0) {
		close_binary(bin);
		return r;
	}

	sym = get_symbol(bin, arraysym);
	if(sym == NULL)
			return -__LINE__;
	r = test_dump_symbol(bin, bits, sym, sym_model);
	free_symbol(sym);

	if(r < 0) {
		close_binary(bin);
		return r;
	}

	r = test_read_zstring_array(bin, arraysym, sym_model);

	close_binary(bin);

	return r;
}

int nulltest()
{
	return open_binary("/path/to/unknown") == NULL ? EXIT_SUCCESS : -__LINE__;
}

int getbits(char *path)
{
	int i = strlen(path);
	if(i > 3 && strncmp(path + strlen(path) - 3, ".so", 3) == 0) {
		i -= 3;
	}
	for(i--; i >= 0 && path[i] != '.'; i--);
	return atoi(path+i+1);
}

char* getname(char *path)
{
	char *name = strdup(path);
	char *bnam = basename(name);
	int i = strlen(bnam);
	if(i > 3 && strncmp(bnam + strlen(bnam) - 3, ".so", 3) == 0) {
		i -= 3;
	}
	for(i--; i >= 0 && bnam[i] != '.'; i--);
	if(i > 0) {
		bnam[i] = 0;
		char *result = strdup(bnam);
		free(name);
		return result;
	}
	free(name);
	return NULL;
}

int main(int argc, char **argv)
{
	int i,r;
	if((r = nulltest())) {
		fprintf(stderr, "Failed nulltest() at line %d\n", -r);
		exit(r);
	}
	for(i = 1; i < argc; i++) {
		char *name = getname(argv[i]);
		if(name != NULL) {
			r = testcase(argv[i], getbits(argv[i]), name, "dummy_symbol", "_stup_dummy42__45642");
			if(r) {
				fprintf(stderr, "Failed for testcase %s at line %d\n", argv[i], -r);
				exit(r);
			}
		}
	}
	exit(EXIT_SUCCESS);
	return 0;
}
