/*
 * pptrace.h -- Public interface of PPTrace. Used by EZTrace
 *
 * - pptrace_prepare_binary(binary): open *binary* and returns an opaque
 * 		 structure that is to be used by all other methods here.
 * - pptrace_load_module(bin, library): add *library* to be an hijack
 * 		 library. This library should have a symbal named
 * 		 __pptrace_hijack_list that is an array of string (char*).
 * 		 Each string contain, separated by a space, the name of symbol to hijack,
 * 		 the name of the symbol in the library to write the original address,
 * 		 and the name of the replacement symbol.
 * 		 *bin* is the opaque structure returned by pptrace_prepare_binary().
 * 		 Returns non-zero on failure.
 *
 * - pptrace_add_preload(bin, library): adds *library* to be a preload library.
 * 		 This library will be added to the preload without doing anything else.
 * 		 *bin* is the opaque structure returned by pptrace_prepare_binary().
 * 		 Returns non-zero on failure.
 *
 * - pptrace_run(void *bin, char **argv, char **envp): executes the binary
 * 		with the given argv and envp (see execve).
 * 		 *bin* is the opaque structure returned by pptrace_prepare_binary().
 * 		 Returns non-zero on failure and free the *bin* structure.
 *
 *  Created on: 2 aožt 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
 
#ifndef PPTRACE_PPTRACE_H
#define PPTRACE_PPTRACE_H

void* pptrace_prepare_binary(char *binary);
int   pptrace_load_module(void *bin, char *library);
int   pptrace_add_preload(void *bin, char *library);
int   pptrace_run(void *bin, char **argv, char **envp);


#endif // PPTRACE_PPTRACE_H
