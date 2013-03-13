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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <inttypes.h>
#include "fxt.h"
#include "fxt-tools.h"
#include "fxt_internal.h"

/* add a pid name, return 1 if new */
int fkt_add_pid(fxt_t fxt, uint64_t pid, const char name[FXT_MAXCOMMLEN+1] )
	{
	struct pid_entry *pidentry,*hashed;

	for( hashed=fxt->pidentries[pid%PIDHASH_ENTRIES];
					hashed && hashed->pid != pid;
					hashed=hashed->next );
	if( hashed )
		{
		memcpy(hashed->name,name,FXT_MAXCOMMLEN+1);
		return 0;
		}

	if( !(pidentry=malloc(sizeof(*pidentry))) )
		{
		perror("couldn't allocate room for new pid");
		exit(EXIT_FAILURE);
		}

	memcpy(pidentry->name,name,FXT_MAXCOMMLEN+1);
	pidentry->pid=pid;
	pidentry->next=fxt->pidentries[pid%PIDHASH_ENTRIES];
	fxt->pidentries[pid%PIDHASH_ENTRIES]=pidentry;
	return 1;
	}

void fkt_remove_pid(fxt_t fxt, uint64_t pid)
	{
	struct pid_entry **entry;
	for(entry=&fxt->pidentries[pid%PIDHASH_ENTRIES]; *entry && (*entry)->pid != pid;
			entry=&(*entry)->next);
	if( !*entry )
		{
		EPRINTF("unknown pid %"PRIu64"\n",pid);
		return;
		}
	*entry=(*entry)->next;
	}

static int recordrunningpid( char *statfilename, char pidname[FXT_MAXCOMMLEN+2+1] )
	{
	FILE *file;
	int res;

	if( !(file = fopen(statfilename,"r")) )
		return 0; /* may happen if it just died */
	res = fscanf(file,"%*d %s",pidname);
	fclose(file);
	return res==1;
	}

int fkt_record_pids( fxt_t fxt, int fd )
	{
	const uint64_t	zero=0;
	uint64_t		pid, tid;
	DIR				*dir,*dir2;
	struct dirent	*dirent,*dirent2;
	char			*ptr;
	char			name[sizeof("/proc/")-1+5+sizeof("/task/")-1+5+sizeof("/stat")-1+1]="/proc/",*task, *c;
	char			pidname[FXT_MAXCOMMLEN+2+1] = {};

	if( !(dir = opendir("/proc")) )
		{
		perror("opening /proc");
		fprintf(stderr,"pids won't be recorded\n");
		}
    else {
		while( (dirent = readdir(dir)) )
		{
		pid = strtol(dirent->d_name, &ptr, 0);
		if( ptr && !*ptr )
			{/* ok, was fully a number */
			name[strlen("/proc/")]='\0';
			strcat(name+strlen("/proc/"),dirent->d_name);
			strcat(name+strlen("/proc/"),"/task");
			task=name+strlen(name);
			if (!(dir2 = opendir(name)))
				{/* 2.4 kernel */
				name[strlen("/proc/")]='\0';
				strcat(name+strlen("/proc/"),dirent->d_name);
				strcat(name+strlen("/proc/"),"/stat");
				if( !recordrunningpid(name,pidname) )
					continue;
				if ((c = strchr(pidname,')')))
						*c = '\0';
				write(fd,&pid,sizeof(pid));
				write(fd,pidname+1,FXT_MAXCOMMLEN+1);
				}
			else
				{
				*task++='/';
				while( (dirent2 = readdir(dir2)) )
					{
					tid = strtol(dirent2->d_name, &ptr, 0);
					if( ptr && !*ptr )
						{/* ok, was fully a number */
						*task='\0';
						strcat(task,dirent2->d_name);
						strcat(task,"/stat");
						if( !recordrunningpid(name,pidname) )
							continue;
						if ((c = strchr(pidname,')')))
								*c = '\0';
						write(fd,&tid,sizeof(tid));
						write(fd,pidname+1,FXT_MAXCOMMLEN+1);
						}
					}
				closedir(dir2);
				}
			}
		}
		closedir(dir);
	}
	/* end of list is a zero pid */
	return write(fd,&zero,sizeof(zero));
	}

int fkt_load_pids(fxt_t fxt, FILE *fstream)
	{
	static int		i;
	struct pid_entry	*entry;
	if( !(entry = malloc(sizeof(*entry))) )
		{
		perror("allocating pid entry");
		return -1;
		}
	if( fread(&entry->pid, sizeof(entry->pid), 1, fstream) < 1)
		{
		perror("reading pid");
		return -1;
		}
	if( !entry->pid )
		{
		free(entry);
		return 0;
		}
	i++;
//	fprintf(stderr,"\r%d",i);
	if( fread(&entry->name, sizeof(entry->name), 1, fstream) < 1 )
		{
		perror("reading pid name");
		exit(EXIT_FAILURE);
		}

	entry->name[FXT_MAXCOMMLEN]='\0';
	entry->next=fxt->pidentries[entry->pid%PIDHASH_ENTRIES];
	fxt->pidentries[entry->pid%PIDHASH_ENTRIES]=entry;
	return fkt_load_pids(fxt, fstream);
	}

const char *fxt_lookup_pid(fxt_t fxt, uint64_t pid)
	{
	struct pid_entry	*cur;
	if( fxt->infos.space == FXT_SPACE_KERNEL && pid <= 0 ) return "idle";
	for( cur=fxt->pidentries[pid%PIDHASH_ENTRIES];
		cur && cur->pid != pid; cur=cur->next);
	if( !cur ) return "unknown";
	else return cur->name;
	}

/* vim: ts=4
 */
