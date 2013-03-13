/*
 *  Copyright (C) 2003, 2004, 2011-2012  Samuel Thibault <samuel.thibault@ens-lyon.org>
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


#include <config.h>
#include <sys/types.h>
#ifndef __MINGW32__
#include <sys/wait.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#ifdef HAVE_LIBBFD
#include <bfd.h>
#endif
#include "fxt.h"
#include "fxt-tools.h"
#include "fxt_internal.h"

#define MAXNAMELEN 128

/* System.map is sorted, but /proc/ksyms is not, so we have to sort out a little
 * bit ourselves */

/* add a symbol, return 1 if new */
int fxt_add_symbol( fxt_t fxt, struct address_entry *entry )
	{
	struct address_entry *cur,*found=NULL;
	struct address_entry **hash;
	unsigned long hashkey;
	const char *c;
	
	for( hashkey=0, c=entry->name; *c; hashkey+=*c, c++);
	hash = &fxt->addresses[hashkey%NAMEHASH_ENTRIES];

	for( cur = *hash; cur; cur = cur->next )
		if( !strcmp(cur->name,entry->name) && cur->source!=FXT_USER
			&& cur->source!=entry->source
			&& (!found || found->address != entry->address) )
		/* if the same name is found in another source, and we didn't already,
		 * or with another address, update the found pointer */
			found = cur;
	if( found )
		{
		if( found->address != entry->address )
			{/* different address and built-in */
				fprintf(stderr,"incoherent symbol %s: %"PRIx64" and %"PRIx64"\n", entry->name,
												entry->address, found->address);
			fprintf(stderr,"please provide the /boot/System.map file of the current running kernel\nthanks to the -S option\n");
			exit(EXIT_FAILURE);
			}
		return 0;
		}
	else
		{
		entry->next = *hash;
		*hash = entry;
		return 1;
		}
	}

static void get_symbolsfd( fxt_t fxt, FILE *fd, fxt_symbol_source source, unsigned long base);

static void add_elfsymbols( fxt_t fxt, const char *path, unsigned long base )
	{
#ifdef HAVE_LIBBFD
	static int			bfd_inited = 0;
	bfd					*abfd;
	long				symcount;
	void				*minisyms,*curminisym;
	unsigned int		size;
	asymbol				*store,*cur;
	struct address_entry	*entry;

	if (!bfd_inited)
		{
		bfd_init();
		bfd_set_default_target("");
		bfd_inited=1;
		}
	if (!(abfd = bfd_openr(path,NULL)))
		/* bad target */
		goto out;
	if (!bfd_check_format(abfd,bfd_object))
		goto out_bfd;
	if (!(bfd_get_file_flags(abfd)&HAS_SYMS))
		goto out_bfd;
	if ((symcount=bfd_read_minisymbols(abfd, 0, &minisyms, &size)) <= 0)
		/* no symbols or error */
		goto out_bfd;

	if (!(store=bfd_make_empty_symbol(abfd)))
		goto out_minisym;

	for (curminisym = minisyms;
		curminisym < minisyms + symcount*size; 
		curminisym += size)
		{
		if (!(cur = bfd_minisymbol_to_symbol(abfd, 0, curminisym, store)))
			{
			bfd_perror("bfd_minisymbol_to_symbol");
			exit(EXIT_FAILURE);
			}
		if (!cur->value || !cur->name || !*(cur->name) || !(cur->flags&BSF_FUNCTION))
			continue;
		if( !(entry = malloc(sizeof(*entry))) )
			{
			fprintf(stderr,"couldn't allocate memory for symbol\n");
			perror("malloc");
			exit(EXIT_FAILURE);
			}
		entry->source = FXT_USER;
		entry->name = strdup(cur->name);

		entry->len = strlen(cur->name);
		entry->address = cur->value + cur->section->vma + base;
		//printf("%s(%p:%s): %#010llx\n+%#010llx\n+%#010lx\n=%#010lx\n",entry->name,cur->section,cur->section->name,cur->value,cur->section->vma,base,entry->address);

		if (!fxt_add_symbol(fxt, entry))
			free(entry);
		}

out_minisym:
	free(minisyms);
out_bfd:
	bfd_close(abfd);
out:
	return;
#endif
	}

void fut_get_mysymbols( fxt_t fxt )
	{
	FILE *maps;
	char attrs[5];
	char path[256];
	unsigned long base,size,offset;
	int n;
	int first=1;
	if (!(maps=fopen("/proc/self/maps","r")))
		{
		perror("opening /proc/self/maps");
		fprintf(stderr,"symbols will be missing\n");
		return;
		}
	while (1)
		{
		if ((n=(fscanf(maps,"%lx-%*s%s%lx%*s%ld",&base,attrs,&offset,&size)))==EOF)
			break;
		if (n==4 && size)
			{
			if ((n=fscanf(maps,"%s",path))==EOF)
				break;
			if (n==1 && attrs[2]=='x')
				add_elfsymbols(fxt,path,first?0:
#ifdef __ia64
#  warning no base add_elfsymbols in fut_get_mysymbols on IA64
#else
								base+
#endif
								offset);
			if (first) first=0;
			}
		do { n=fgetc(maps); }
		while( n!=EOF && n!='\n' );
		}
	fclose(maps);
	}

#ifndef __MINGW32__
void fkt_addmodule_symbols( fxt_t fxt, struct module_entry *module )
	{
	int fd[2];
	FILE *ffd;
	char *d;
	pid_t				pid;
	/* try to nm the module */
	if( (d = strstr(module->path,".o_M")) )
		*(d+2)='\0';	/* strip module version */

	fprintf(stderr,"\rnming %s\n",module->path);
	if( pipe(fd) )
		{
		perror("couldn't make a pipe for nm\n");
		exit(EXIT_FAILURE);
		}
	if( !(pid=fork()) )
		{/* exec nm on this file */
		close(fd[0]);
		close(STDOUT_FILENO);
		if( dup2(fd[1],STDOUT_FILENO) < 0 )
			{
			perror("couldn't redirect output of nm\n");
			exit(EXIT_FAILURE);
			}
		close(fd[1]);
		execlp("nm", "nm", module->path, NULL);
		fprintf(stderr,"couldn't launch nm on %s\n",module->path);
		perror("execlp");
		exit(EXIT_FAILURE);
		}

	close(fd[1]);
	if( !(ffd=fdopen(fd[0],"r")) )
		/* couldn't open nm output, give up */
		kill(pid,SIGTERM);
	else
		get_symbolsfd(fxt, ffd, FXT_SYSMAP_FILE, module->base);
	wait(NULL);
	}
#endif

static void get_symbolsfd( fxt_t fxt, FILE *fd, fxt_symbol_source source, unsigned long base)
	{
	int					i=0,j,n;
	char				*c;
	struct address_entry	*entry=NULL;
	struct module_entry	*module;
	char				*name;
	unsigned long		address;
	int					len;
	char				type;
	
	for(;;i++)
		{
		//fprintf(stderr,"\r%d",i);
		if( !(name = malloc(MAXNAMELEN)) )
			{
			fprintf(stderr,"couldn't allocate memory for name\n");
			perror("malloc");
			exit(EXIT_FAILURE);
			}

		type=0;
		if( (n = fscanf(fd, "%lx ", &address)) == EOF
			|| (n > 0 && (n = fscanf(fd, "%s%n", name, &len)) == EOF)
			|| (n > 0 && len<=1 && (type=name[0],
					n = fscanf(fd, "%s%n", name, &len)) == EOF ) )
			{/* end of file, stop */
			//fprintf(stderr," done\n");
			return;
			}

		do { j=fgetc(fd); }
		while( j!=EOF && j!='\n' );

		if( !n || (type != 0 && type != 't' && type != 'T') )
			/* garbage or non text, continue */
			{
			free(name);
			continue;
			}

		for( c=name; c<name+len && *c!=' '; c++);
		len=c-name;
		name[len]='\0';

		if( source != FXT_MODLIST )
			{
			if( !(entry = malloc(sizeof(*entry))) )
				{
				fprintf(stderr,"couldn't allocate memory for symbol\n");
				perror("malloc");
				exit(EXIT_FAILURE);
				}
			entry->source = source;
			entry->name = name;
			entry->len = len;
			entry->address = address + base;
		}
		
#ifndef __MINGW32__
		/* look for module insertion tags */
		if( !strncmp(name,"__insmod_",strlen("__insmod_")) )
			{
			if( source != FXT_MODLIST )
				{/* but ignore them if not a module list */
				free(name);
				continue;
				}
			if( (c = strstr(name,"_O/")) )
				{/* module load tag, record it */
				/* look for text address to add the module path */
				for( module=fxt->modules_entries; module &&
							(strncmp(module->name,name,c-name)
							||strncmp(module->name+(c-name),"_S.text_L",
									strlen("_S.text_L")));
							module = module->next);
				if( module )
					{
					free(module->name);
					module->name = name + strlen("__insmod_");
					module->path = c+2;
					fkt_addmodule_symbols(fxt, module);
					}
				else
					{
					if( !(module=malloc(sizeof(*module))) )
						{
						perror("couldn't allocate module entry\n");
						exit(EXIT_FAILURE);
						}
					module->name = name;
					module->path = c+2;
					module->next = fxt->modules_entries;
					fxt->modules_entries = module;
					}
				}
			else if( (c = strstr(name,"_S.text_L")) )
				{/* module text address */
				/* look for module load tag to add the base address */
				for( module = fxt->modules_entries; module &&
							(strncmp(module->name,name,c-name)
							||strncmp(module->name+(c-name),"_O/",
									strlen("_O/")));
							module = module->next);
				if( module )
					{
					module->base = address;
					free(name);
					fkt_addmodule_symbols(fxt, module);
					}
				else
					{
					if( !(module=malloc(sizeof(*module))) )
						{
						perror("couldn't allocate module entry\n");
						exit(EXIT_FAILURE);
						}
					module->name = name;
					module->base = address;
					module->next = fxt->modules_entries;
					fxt->modules_entries = module;
					}
				}
			}
		else
#endif
			/* only add symbol if not module list */
			if( source != FXT_MODLIST && !fxt_add_symbol(fxt, entry) )
				free(entry);
		}
	}

void fxt_get_symbols( fxt_t fxt, const char *file, fxt_symbol_source source, unsigned long base )
	{
	FILE *fd;
	if( !file ) return;
	//fprintf(stderr,"opening %s\n", file);
	if( (fd = fopen(file, "r")) < 0 )
		{
		fprintf(stderr,"opening %s\n",file);
		perror("open");
		exit(EXIT_FAILURE);
		}
	get_symbolsfd( fxt, fd, source, base );
	fclose(fd);
	}

int fxt_record_symbols( fxt_t fxt, int fd )
	{
	int i;
	const uint64_t zero=0;
	struct address_entry *cur;
	for( i=0; i<NAMEHASH_ENTRIES; i++ )
		for( cur = fxt->addresses[i]; cur; cur=cur->next )
			{
#ifdef __ia64
#  warning all adresse valid...
#else
			if (cur->address<0x00400000)
				fprintf(stderr,"strange symbol %s: %"PRIx64"\n",cur->name,cur->address);
			else
#endif
			if (cur->len>1000)
				fprintf(stderr,"strange name length: %s: %x\n",cur->name,cur->len);
			else
				{
				write(fd,&cur->address,sizeof(cur->address));
				write(fd,&cur->len,sizeof(cur->len));
				write(fd,cur->name,cur->len);
				}
			}
	/* end of list is a zero address */
	write(fd,&zero,sizeof(zero));
	return 0;
	}

int fxt_load_symbols( fxt_t fxt, FILE *fstream )
	{
	//static int i;
	struct address_entry	*entry;
	char *name;
restart:
	if( !(entry = malloc(sizeof(*entry))) )
		{
		perror("allocating System map entry");
		return -1;
		}
	READ(64,entry->address);
	if( !entry->address )
		{
		free(entry);
		return 0;
		}
//	if (!((i++)%1024))
//		fprintf(stderr,"\r%d",i);
	READ(32,entry->len);
	if( !(name = malloc(entry->len+1)) )
		{
		perror("allocating System map entry name");
		return -1;
		}
	if( entry->len && fread(name, 1, entry->len, fstream) < entry->len )
		{
		perror("reading System map name");
		return -1;
		}

	name[entry->len]='\0';
	entry->name=name;
	entry->next=fxt->addresses[entry->address%NAMEHASH_ENTRIES];
	fxt->addresses[entry->address%NAMEHASH_ENTRIES]=entry;
	//printf("Registering symbol %lx=%s\n", entry->address, entry->name);
	goto restart;
	}

char nosymb[256];
const char *fxt_lookup_symbol( fxt_t fxt, unsigned long address )
	{
	struct address_entry	*cur;
	for( cur=fxt->addresses[address%NAMEHASH_ENTRIES];
		cur && cur->address != address; cur=cur->next);
	if( !cur ) {
		snprintf(nosymb, 255, "No symbol at %lx", address);
		return nosymb;
	}
	return cur->name;
	}

/* vim: ts=4
 */
