/*
 * hijack.h -- High-level interface for inserting hijacks into a traced process
 *
 * Provided methods are:
 * 	- hijack_code(bin, child, sym_addr, sym_size, reloc_addr, orig_addr,
 * 		repl_addr): hijacks the symbol at address *sym_addr* which size is
 * 			*sym_size*. It will insert a jump to *repl_addr* at *sym_addr*. The
 * 			symbol at *orig_addr* will be filled with the address *reloc_addr*
 * 			that contains the code overriden by the trampoline. This is done by
 * 			determining the size of instruction using the *bin* parameter (a
 * 			binary opened by open_binary())
 *  - hijack(bin, child, toHijack, orig, repl): allocates
 *  	the destination buffer for the hijack and then do the hijack using
 *  	hijack_symbol(). *toHijack* is the symbol to intercept, *orig* is the
 *  	symbol to store the address of the hijack code and *repl* is the symbol
 *  	called instead of the *toHijack* symbol
 *
 *  Created on: 2 juil. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */

#ifndef HIJACK_H_
#define HIJACK_H_


ssize_t hijack_code(void* bin, pid_t child, uint64_t sym_addr,
		uint64_t sym_size, uint64_t reloc_addr,
		uint64_t orig_addr, uint64_t repl_addr);
ssize_t hijack(void* bin, pid_t child, zzt_symbol *toHijack,
		zzt_symbol *orig, zzt_symbol *repl);

#endif /* HIJACK_H_ */
