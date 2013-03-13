/*
*  Copyright (C) 2004, 2011-2012 Samuel Thibault <samuel.thibault@ens-lyon.org>
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

#ifndef __FXT_INTERNAL_H
#define __FXT_INTERNAL_H

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include "fxt-tools.h"

struct fxt_block {
	off_t start;
	struct fxt_block *parent;
	struct fxt_block_ondisk {
		uint64_t size;
		int32_t type;
		int32_t subtype;
	} ondisk ;
};

extern struct fxt_block *__fxt_block_current;

/******************************************************************************
 *
 * opened trace structure
 */
struct code_list_item {
	unsigned long long	code;
	const char		*name;
	struct code_list_item	*link;
};

/* names are hashed according to the first 5 low bits of the 2 first characters,
 * i.e. 1024 entries */
#define NAMEHASH_ENTRIES 1024
struct address_entry {
	const char *name;
	uint32_t len;
	uint64_t address;
	fxt_symbol_source source;
	struct address_entry *next;
};
struct module_entry {
	char *name;
	unsigned long base;
	const char *path;
	struct module_entry *next;
};

#define PIDHASH_ENTRIES 256
struct pid_entry {
	uint64_t pid;
	char name[FXT_MAXCOMMLEN+1];
	struct pid_entry *next;
};

struct _fxt {
	/* first and uname block: misc informations */
	struct fxt_infos infos;

	/* irq block */
	unsigned int nirqs;
	struct code_list_item *irq_list;

	/* addresses block */
	struct address_entry *addresses[NAMEHASH_ENTRIES];
	struct module_entry *modules_entries;

	/* pids block */
	struct pid_entry *pidentries[PIDHASH_ENTRIES];

	/* opened file */
	int fd;
        FILE* fstream;
	struct fxt_block events_block;

	/* lwp status */
	int nlwps;
	int *already_saw;
	uint64_t *secondlasttid;
	uint64_t *lasttid;
};

/******************************************************************************
 *
 * Names
 */

extern int fkt_record_irqs(fxt_t fxt, int fd);
extern int fkt_load_irqs(fxt_t fxt, FILE *fstream);

extern int fkt_record_pids(fxt_t fxt, int fd);
extern int fkt_load_pids(fxt_t fxt, FILE *fstream);

extern int fxt_record_symbols(fxt_t fxt, int fd);
extern int fxt_load_symbols(fxt_t fxt, FILE *fstream);

/******************************************************************************
 * 
 * Record facilities
 */

extern int fxt_get_cpu_info(fxt_t fxt);

/******************************************************************************
 *
 * Blocks management
 */
#define FXT_ALIGN 8

/* this is used to enforce the record file structure, with an offset field at
 * the beginning of every block, which tells the size of the block and then an
 * int describing the type of the block */

#define GET_BLOCK_TYPE(block) (block).ondisk.type
#define GET_BLOCK_SUBTYPE(block) (block).ondisk.subtype


inline static off_t fxt_block_align(int fd) {
	off_t start;
	if( (start=lseek(fd,0,SEEK_CUR)) < 0 ) {
		perror("lseek getting current position");
		exit(EXIT_FAILURE);
	}
#if 0
        if ( start & (FXT_ALIGN-1) ) {
		if ( (start=lseek(fd,
				FXT_ALIGN-(start & (FXT_ALIGN-1)),
				SEEK_CUR)) < 0) {
			perror("lseek aligning position");
			exit(EXIT_FAILURE);
		}
	} 
#endif
	return start;
}

inline static off_t fxt_block_falign(FILE *fstream) {
	off_t start;
	if( (start=ftell(fstream)) < 0 ) {
		perror("ftell getting current position");
		exit(EXIT_FAILURE);
	}
#if 0
        if ( start & (FXT_ALIGN-1) ) {
		if ( (fseek(fstream,
				FXT_ALIGN-(start & (FXT_ALIGN-1)),
				SEEK_CUR)) < 0) {
			perror("lseek aligning position");
			exit(EXIT_FAILURE);
		}
		start = ftell(fstream);
	} 
#endif
	return start;
}

#define ENTER_FBLOCK(fstream) \
	{ \
		struct fxt_block __fxt_block; \
        	fxt_fblock_enter(fxt, fstream, &__fxt_block);

inline static void fxt_fblock_enter(fxt_t fxt, FILE *fstream, struct fxt_block *block)
{
	block->start=fxt_block_falign(fstream);
	block->parent=__fxt_block_current;
	if( fread(&block->ondisk, sizeof(struct fxt_block_ondisk), 1, fstream) < 1 )
	{
		perror("entering block");
		exit(EXIT_FAILURE);
	}
	SWAP(64,block->ondisk.size);
	SWAP(32,block->ondisk.type);
	SWAP(32,block->ondisk.subtype);
	if (0==block->ondisk.size && block->parent && block->parent->ondisk.size) {
		block->ondisk.size=block->parent->ondisk.size;
	}
	__fxt_block_current=block;
}

#define ENTER_FBLOCK_TYPE(fstream, type) \
        ENTER_FBLOCK(fstream); \
        fxt_block_ensure_type(&__fxt_block, type);

#define ENTER_BLOCK(fd) \
	{ \
		struct fxt_block __fxt_block; \
        	fxt_block_enter(fxt, fd, &__fxt_block);

inline static void fxt_block_enter(fxt_t fxt, int fd, struct fxt_block *block)
{
	block->start=fxt_block_align(fd);
	block->parent=__fxt_block_current;
	if( read(fd,&block->ondisk, sizeof(struct fxt_block_ondisk)) < (ssize_t) sizeof(struct fxt_block_ondisk) )
	{
		perror("entering block");
		exit(EXIT_FAILURE);
	}
	SWAP(64,block->ondisk.size);
	SWAP(32,block->ondisk.type);
	SWAP(32,block->ondisk.subtype);
	if (0==block->ondisk.size && block->parent && block->parent->ondisk.size) {
		block->ondisk.size=block->parent->ondisk.size;
	}
	__fxt_block_current=block;
}

#define ENTER_BLOCK_TYPE(fd, type) \
        ENTER_BLOCK(fd); \
        fxt_block_ensure_type(&__fxt_block, type);

inline static void fxt_block_ensure_type(struct fxt_block *block, int type)
{
	if (block->ondisk.type != type) {
		fprintf(stderr, "Should entering block type %i, but loading type %i\n",
			type, block->ondisk.type);
		exit(EXIT_FAILURE);
	}
}

#define LEAVE_BLOCK(fd) \
		fxt_block_leave(fd, &__fxt_block); \
	}	/* matching {} */

inline static void fxt_block_leave(int fd, struct fxt_block *block)
{
	off_t end=block->start + block->ondisk.size;
	if (block->ondisk.size==0) {
		/* special value 0: block spans up to the end of file */
		if (lseek(fd, 0, SEEK_END) < 0) {
			perror("seeking forward to end of block");
			exit(EXIT_FAILURE);
		}
	} else {
		if (lseek(fd, end, SEEK_SET) != end ) {
			perror("seeking forward to end of block");
			exit(EXIT_FAILURE);
		}
	}
	__fxt_block_current=block->parent;
}

#define LEAVE_FBLOCK(fstream) \
		fxt_fblock_leave(fstream, &__fxt_block); \
	}	/* matching {} */

inline static void fxt_fblock_leave(FILE *fstream, struct fxt_block *block)
{
	off_t end=block->start + block->ondisk.size;
	if (block->ondisk.size==0) {
		/* special value 0: block spans up to the end of file */
		if (fseek(fstream, 0, SEEK_END) < 0) {
			perror("seeking forward to end of block");
			exit(EXIT_FAILURE);
		}
	} else {
		if (fseek(fstream, end, SEEK_SET) != 0 ) {
			perror("seeking forward to end of block");
			exit(EXIT_FAILURE);
		}
	}
	__fxt_block_current=block->parent;
}

#define BEGIN_BLOCK(fd, type) \
	{ \
                 struct fxt_block __fxt_block; \
                 fxt_block_begin(fd, type, 0, 0, &__fxt_block);

inline static void fxt_block_begin(int fd, int type, int subtype, off_t size, struct fxt_block *block)
{
	block->start=fxt_block_align(fd);
	block->ondisk.size=size;
	block->ondisk.type=type;
	block->ondisk.subtype=subtype;
	if( write(fd,&block->ondisk,sizeof(block->ondisk)) != sizeof(block->ondisk) ) {
		perror("writing dummy offset");
		exit(EXIT_FAILURE);
	}
	block->parent=__fxt_block_current;
	__fxt_block_current=block;
}

#define END_BLOCK(fd)	\
                fxt_block_end(fd, &__fxt_block); \
        }

inline static void fxt_block_end(int fd, struct fxt_block *block)
{
	off_t end;
	if( (end=lseek(fd,0,SEEK_CUR)) < 0 ) {
		perror("getting block end seek");
		exit(EXIT_FAILURE);
	}
	if( lseek(fd,block->start,SEEK_SET) != block->start ) {
		perror("seeking back to beginning of block");
		exit(EXIT_FAILURE);
	}
	block->ondisk.size = end - block->start;
	if( write(fd,&block->ondisk.size,sizeof(block->ondisk.size)) < (ssize_t) sizeof(block->ondisk.size) ) {
		perror("writing end of block size\n");
		exit(EXIT_FAILURE);
	}
	fxt_block_leave(fd, block);
}

inline static void skip_block( fxt_t fxt )
{
	off_t size;
	if( fread(&size,sizeof(size),1,fxt->fstream) < 1 ) {
		perror("reading end of block size\n");
		exit(EXIT_FAILURE);
	}
	SWAP(32,size);
	if (size==0) {
		if (fseek(fxt->fstream,0,SEEK_END)<0) {
			perror("seeking forward to end of block");
			exit(EXIT_FAILURE);
		}
	}
	if( fseek(fxt->fstream,size-sizeof(size),SEEK_CUR) < 0 ) {
		perror("seeking forward to end of block");
		exit(EXIT_FAILURE);
	}
}

enum {
	FXT_BLOCK_INFOS=1,

	FXT_BLOCK_TIME=0x10,
	FXT_BLOCK_UNAME,
	FXT_BLOCK_IRQS,
	FXT_BLOCK_ADDRS,
	FXT_BLOCK_PIDS,

	FXT_BLOCK_TRACES_KERNEL_RAW32=0x100,
	FXT_BLOCK_TRACES_KERNEL_RAW64,
	FXT_BLOCK_TRACES_USER_RAW32=0x200,
	FXT_BLOCK_TRACES_USER_RAW64,
};

unsigned long* __fut_record_event(fxt_trace_user_raw_t * rec, uint64_t stamp, unsigned long code);
int fut_flush( char* filename, void* next_slot, int record_flush_events );
void __fut_reset_pointers();

#endif /* __FXT_INTERNAL_H */
