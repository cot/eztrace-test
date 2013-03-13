/*
 *  Copyright (C) 2010  Samuel Thibault <samuel.thibault@ens-lyon.org>
 *
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY ; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the program ; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* This is a very trivial working example that uses fxt and produces a trace
 * file. Build with gcc example.c -o example -lfxt */

#define CONFIG_FUT

#include <fxt/fxt.h>
#include <fxt/fut.h>
#include <pthread.h>

/* Choose 1MB buffer size, enable the whole tracing mask, use pthread
 * thread ID */

#define THREAD_ID pthread_self()
int main(void) {
	fut_setup(1<<20, FUT_KEYMASKALL, THREAD_ID);

	FUT_PROBE1(-1, 0x1000, 1);
	FUT_PROBE1(-1, 0x1001, 1);

	fut_endup("fichier");
	fut_done();
	return 0;
}
