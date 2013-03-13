/*
 * binary.h -- Interface for reading symbol from binaries
 *
 * Provided methods:
 * 	  - open_binary(path): open the binary for reading and return an opaque
 * 	  					structure
 * 	  - close_binary(bin): close the opaque structure opened by open_binary
 * 	  - get_symbol(bin, symbol): get the symbol in which name is provided by
 * 	  					the symbol parameter from the binary bin (opaque structure).
 * 	  					The returned structure contains the section of the symbol (name
 * 	  					and address), the offset of the symbol inside the section,
 * 	  					the symbol name and, if available, the symbol size.
 * 	  - free_symbol(symbol): free a structure returned by a previous call of get_symbol()
 * 	  - get_binary_bits(bin): return the size of a word inside the binary (e.g. is the code
 * 	  					of the binary for a 32 bits arch or a 64 bits
 *    - read_symbol(bin, symbol, buffer, size): read the symbol content from the binary bin.
 *                      If the symbol is found and present in the library, then its content is
 *                      copied in buffer copying at most size bytes. The number of copied bytes
 *                      are returned in result.
 *    - read_zstring(bin, symbol, addr, buffer, size): read size bytes given an address read from
 *    					the symbol given in argument. If the address is found, at most size bytes
 *    					are copied into buffer.
 *    - read_zstring_array(bin, symbol): read a null terminated string array from a binary and
 *    					allocate the string array. The string array should be freed upon with
 *    					free_zstring_array().
 *    - free_zstring_array(arr): delete a null terminated string array allocated by read_zstring_array().
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#ifndef PPTRACE_BINARY_H_
#define PPTRACE_BINARY_H_
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

typedef uint64_t zzt_word;

typedef struct _zzt_symbol {
	zzt_word   symbol_offset;
	char      *symbol_name;
	zzt_word   symbol_size;
	char      *section_name;
	zzt_word   section_addr;
} zzt_symbol;

#define INIT_ZZT_SYMBOL(n, addr, size) zzt_symbol n; n.symbol_name = #n; n.symbol_size = (zzt_word)(size); n.section_name = NULL; n.section_addr = (zzt_word)0; n.symbol_offset = (zzt_word)(addr);

void close_binary(void *bin);
void* open_binary(char *path);
zzt_symbol* get_symbol(void *bin, char *symbol);
void free_symbol(zzt_symbol *symbol);
int get_binary_bits(void *bin);
size_t read_symbol(void *bin, zzt_symbol *symbol, void *buffer, ssize_t size);
size_t read_zstring(void *bin, zzt_symbol *symbol, zzt_word addr, void *buffer, ssize_t size);
char** read_zstring_array(void *bin, char *symbol);
void free_zstring_array(char **strarray);

#endif /* PPTRACE_BINARY_H_ */
