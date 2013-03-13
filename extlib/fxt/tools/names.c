/*	names.c

	names -- names of system calls, traps, irqs and functions

	Copyright (C) 2000, 2001  Robert D. Russell -- rdr@unh.edu

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*****	15-Oct-00	rdr		modified for 2.4.0-test9 *****/
/*	also commented out use of FKT_TCP_RECVMSG_SCHEDI_CODE */

#include <config.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/fkt.h>
#include "fxt.h"
#include "fxt_internal.h"

/*
 * IRQs
 */

#define LINE_SIZE	128

#ifndef WHITE_SPACE
#define WHITE_SPACE	" \t\n\f\v\r"
#endif

/*	reads /proc/interrupts to find out the IRQ assignment on this machine */
/*	also figures out number of CPUs */
int fkt_fill_irqs( fxt_t fxt )
	{
	FILE					*table;
	struct code_list_item	*ptr;
	char					*lptr, *xptr, line[LINE_SIZE], *name;
	unsigned int			n, i, ncpus;


	fxt->nirqs = 0;
	ncpus = 0;
	if( (table = fopen("/proc/interrupts", "r")) == NULL )
		{
		fprintf(stderr, "unable to open /proc/interrupts\n");
		return -1;
		}

	/*	first line of file is headers of form CPUn */
	if( fgets(line, LINE_SIZE, table) == NULL )
		{
		fprintf(stderr, "unable to read /proc/interrupts\n");
		fclose(table);
		return -1;
		}
	
	for( lptr = line;  strtok(lptr, WHITE_SPACE) != NULL;  ncpus++ )
		lptr = NULL;
	/*
	 * not useful any more
	printf("       : ncpus %5u\n", ncpus);
	*/

	/*	remaining lines of file are the IRQs */
	while( fgets(line, LINE_SIZE, table) != NULL )
		{/* build circular list with 1 item for each interrupt */
		n = strtoul(line, &lptr, 10);	/* scan off leading number */
		if( *lptr != ':' )				/* which must end with : */
			continue;					/* or it is not an IRQ number */
		ptr = (struct code_list_item *)malloc(sizeof(struct code_list_item));
		ptr->code = n;
		
		/*	scan over the interrupt counts, one per cpu, and controller type */
		xptr = lptr;
		for( lptr++, i=0;  i<=ncpus && (xptr = strtok(lptr, WHITE_SPACE))!=NULL;
																		i++ )
			lptr = NULL;

		/*	get to the leading character of the interrupt name */
		xptr += strlen(xptr) + 1;
		xptr += strspn(xptr, WHITE_SPACE);

		i = strlen(xptr);
		name = malloc(i);
		strncpy(name, xptr, i);
		name[i-1] = '\0';
		ptr->name = name;
		if( fxt->irq_list == NULL )
			ptr->link = ptr;
		else
			{
			ptr->link = fxt->irq_list->link;
			fxt->irq_list->link = ptr;
			}
		fxt->irq_list = ptr;
		/*
		printf(" irq %2u: %s\n", ptr->code, ptr->name);
		*/
		fxt->nirqs++;
		}
	/*
	printf("       : nirqs %4u\n", fxt->nirqs);
	printf("       : ncpus %4u\n", ncpus);
	*/

	fclose(table);

	if( (ptr = fxt->irq_list) != NULL )
		{/* make circular list into singly-linked, NULL-terminated list */
		fxt->irq_list = fxt->irq_list->link;
		ptr->link = NULL;
		}
	return 0;
	}

int fkt_record_irqs(fxt_t fxt, int fd)
	{
	struct code_list_item	*ptr;
	unsigned int i;

	write(fd, (void *)&fxt->nirqs, sizeof(fxt->nirqs));
	write(fd, (void *)&fxt->infos.ncpus, sizeof(fxt->infos.ncpus));
	for( ptr = fxt->irq_list;  ptr != NULL;  ptr = ptr->link )
		{
		write(fd, (void *)&ptr->code, sizeof(ptr->code));
		i = strlen(ptr->name)+1;
		write(fd, (void *)&i, sizeof(i));
		i = (i + 3) & ~3;
		write(fd, ptr->name, i);
		}
	return 0;
	}

int fkt_load_irqs( fxt_t fxt, FILE *fstream )
	{
	struct code_list_item	*ptr;
	unsigned int		i, k, n;
	unsigned int			ncpus;
	char					*name;


	if( fread((void *)&fxt->nirqs, sizeof(fxt->nirqs), 1, fstream) <= 0 )
		{
		perror("read nirqs");
		return -1;
		}
	//printf("%14s = %u\n", "nirqs", fxt->nirqs);
	if( fread((void *)&ncpus, sizeof(ncpus), 1, fstream) <= 0 )
		{
		perror("read ncpus");
		return -1;
		}
	//printf("%14s = %u\n", "ncpus", ncpus);

	for( n = 0;  n < fxt->nirqs;  n++ )
		{/* build circular list with 1 item for each interrupt */
		ptr = (struct code_list_item *)malloc(sizeof(struct code_list_item));
		fread((void *)&ptr->code, sizeof(ptr->code), 1, fstream);
		fread((void *)&i, sizeof(i), 1, fstream);
		k = (i + 3) & ~3;
		name = malloc(k);
		fread((void *)name, 1, k, fstream);
		name[i-1] = '\0';
		ptr->name = name;
		if( fxt->irq_list == NULL )
			ptr->link = ptr;
		else
			{
			ptr->link = fxt->irq_list->link;
			fxt->irq_list->link = ptr;
			}
		fxt->irq_list = ptr;
		//printf("irq %2llu: %s\n", ptr->code, ptr->name);
		}
	//printf("\n");
	if( (ptr = fxt->irq_list) != NULL )
		{/* make circular list into singly-linked, NULL-terminated list */
		fxt->irq_list = fxt->irq_list->link;
		ptr->link = NULL;
		}
	return 0;
	}

const char *fkt_find_irq( fxt_t fxt, unsigned long code )
	{
	struct code_list_item	*ptr;
	static char buf[32];

	for( ptr = fxt->irq_list;  ptr != NULL;  ptr = ptr->link )
		{
		if( ptr->code == code )
			return ptr->name;
		}
	sprintf(buf,"unknown irq %lu",code);
	return buf;
	}

/*
 * Traps, sysIRQs and syscalls
 */

const char	*fkt_i386_traps[FKT_I386_NTRAPS] = {
#include "fxt/trap_names.h"
};

const char	*fkt_i386_sysirqs[FKT_I386_NSYSIRQS] = {
#include "fxt/sysirq_names.h"
};

const char	*fkt_i386_syscalls[FKT_I386_NSYSCALLS] = {
#include "fxt/syscall_names.h"
};

int fkt_find_syscall( fxt_t fxt, unsigned long code, char *category, const char **name, int *should_be_hex )
	{
	int		z;

	*should_be_hex = 0;
	if( code <= FKT_SYS_CALL_MASK )
		if ( code == FKT_LCALL7 )
			{
			z = 0x7;
			*should_be_hex = 1;
			*category = 'S';
			*name = "lcall7";
			}
		else if ( code == FKT_LCALL27 )
			{
			z = 0x27;
			*should_be_hex = 1;
			*category = 'S';
			*name = "lcall27";
			}
		else
			{/* this is a true system call */
			z = code;
			*should_be_hex = 1;
			*category = 'S';
			*name = fkt_i386_syscalls[z];
			}
	else if( code < FKT_TRAP_LIMIT_CODE )
		{/* this is a trap */
		z = code - FKT_TRAP_BASE;
		if( z > FKT_I386_NTRAPS )
			z = FKT_I386_NTRAPS;
		*category = 'T';
		*name = fkt_i386_traps[z];
		}
	else if ( code < FKT_IRQ_SYS )
		{/* device IRQ */
		z = code - FKT_IRQ_TIMER;
		*category = 'I';
		*name = fkt_find_irq(fxt, z);
		}
	else
		{/* system IRQ */
		z = code - FKT_IRQ_SYS;
		*should_be_hex = 1;
		*category = 'I';
		*name = fkt_i386_sysirqs[z];
		z += 0xef;
		}
	return z;
	}

/*
 * kernel functions
 */

struct fxt_code_name	fkt_code_table[] = {
#include "fkt_code_name.h"

	{0, NULL}
};

const char *fxt_find_name( fxt_t fxt, unsigned long code, int keep_entry, int maxlen, struct fxt_code_name *table)
	{
	struct fxt_code_name	*ptr;
	static char			local_buf[128];
	const char				*name=NULL;
	int					len, elen;
	if (!table && fxt->infos.space == FXT_SPACE_KERNEL)
		table = fkt_code_table;

	if( code >= FKT_I386_FUNCTION_MINI )
		name = fxt_lookup_symbol(fxt, code);
	else
		for( ptr = table;  ptr->code != 0;  ptr++ )
			if( ptr->code == code )
				{
				name = ptr->name;
				break;
				}
	if( !name )
		{
		sprintf(local_buf,"unknown code %lx",code);
		name = local_buf;
		}
	if( !keep_entry )
		{/* caller wants _entry stripped off end of name */
		len = strlen(name);
		elen = strlen("_entry");
		if( len > elen  && strcmp(&name[len-elen], "_entry") == 0 )
			{
			strcpy(local_buf, name);
			local_buf[len-elen] = '\0';
			name = local_buf;
			}
		}
	if( maxlen > 0 && strlen(name) > maxlen )
		{
		if( name != local_buf )
			{
			strcpy(local_buf, name);
			name = local_buf;
			}
		local_buf[maxlen] = '\0';
		}
	return name;
	}

/* vi: ts=4
 */
