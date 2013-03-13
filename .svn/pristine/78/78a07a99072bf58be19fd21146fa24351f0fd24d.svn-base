#	fkt_header.S
#
#
# Copyright (C) 2000, 2001  Robert D. Russell -- rdr@unh.edu
#               2003, 2004  Samuel Thibault -- samuel.thibault@ens-lyon.org
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
###	13-Jun-03	st		adapted to new fkt buffering
###	08-Nov-00	rdr		change SMP handling to avoid spin locks
###	11-Jul-00	rdr		change __SMP__ to CONFIG_SMP for 2.4.0
#
#	fkt = Fast Kernel Tracing
#
#	uses 1 external:
#		unsigned fkt_active;		0 when inactive, non-zero when active
#
#
#	call is:	fkt_header( header, p1, p2, ... );
#	where:
#		header low byte has total number of bytes in this probe entry
#			= (3 + number_of_parameters)*4
#		header high 3 bytes have entry identification code
#
#
#	format of one probe record in buffer is:
#		int 1		low 32-bits of cycle clock
#		int 2		low half gets 16-bit pid of current process
#					high half gets 16-bit cpu number
#		int 3		full 32-bit header from caller
#		int 4		full 32-bit p1 from caller
#		int 5		full 32-bit p2 from caller
#		...		...
#		int 3+n		full 32-bit pn from caller
#
#
#	assumptions (and invariants):
#		value in fkt_next_slot is a multiple of 4, grows up within a page
#		value in fkt_cur_data is a multiple of 4
#		value in low byte of header is multiple of 4
#
#
#
#	fkt_active contains the currently active set of probe keys.
#	bit x in fkt_active is 1 if probes in keyset x are active.
#	fkt_active is set  when tracing is actived by fkt_setup();  it is reset
#	to 0 when tracing is deactived by fkt_endup().
#	fkt_active is also reset to 0 by the first thread that fills the buffer.
#

#include <linux/version.h>
#include <linux/linkage.h>
#include <asm/page.h>
#include <linux/fkt.h>
/* at most */
#if (LINUX_VERSION_CODE >= (KERNEL_VERSION(2,6,7)))
#include <asm/asm_offsets.h>
/* at most */
#elif (LINUX_VERSION_CODE >= (KERNEL_VERSION(2,6,0)))
#include <asm/thread_info.h>
#endif

#undef PAGE_SIZE
#define	PAGE_SIZE	(1 << PAGE_SHIFT)
#undef PAGE_MASK
#define PAGE_MASK	(~(PAGE_SIZE-1))

#define STRUCT_DATA_PAGE_SIZE	8

#	Offsets into struct task_struct -- must agree with <linux/sched.h>
#	CAREFUL! these tend to change from version to version
#	since they are NOT wired down!
#
#	these three lines is the result of this program, compiled with -D__KERNEL__  -I/usr/src/linux-.../include
#	and -I/usr/src/linux-.../include/asm-i386/mach-generic on 2.6
#	#include <linux/sched.h>
#	int main(void) {
#		/* to comment on 2.6 kernels */
#		printf("processor\t= %d\n",&((struct task_struct *)0)->processor);
#		/* always correct */
#		printf("pid\t= %d\n",&((struct task_struct *)0)->pid);
#	}

processor	= 48
/* at most */
#if (LINUX_VERSION_CODE >= (KERNEL_VERSION(2,6,10)))
pid		= 148
/* at most */
#elif (LINUX_VERSION_CODE >= (KERNEL_VERSION(2,6,3)))
pid		= 136
#else
pid		= 124
#endif

#	Offsets into caller stack when in these routines
caller_code	= 8			# when data page addr not pushed on TOS
caller_offset	= 12			# after data page addr is pushed on TOS

#	Offsets into probe record
probe_cycles	= 0
probe_pid	= 4
probe_cpu_no	= 6
probe_code	= 8
probe_param1	= 12

#	borrowed from entry.S
#	points reg at current task control block (TCB) for 2.4 kernels,
#	or at current thread_info struct
#define GET_CURRENT(reg) \
	movl	%esp, reg; \
	andl	$-8192, reg

FKT_SOFTIRQ = 4		# *must be the same as in include/linux/interrupt.h*

#
# These are pointers to the data page
#
#		unsigned *fkt_next_slot;	pointer to next free slot in page
#		unsigned long **fkt_cur_data;	pointer to pointer to next page address
#
# They must remain grouped in this order for cmpxchg8b
# fkt_cur_data is considered as hi part, hence second in memory
#
	.bss
	.align 4

.globl fkt_next_slot
	.size	fkt_next_slot,4
fkt_next_slot:
	.zero	4
.globl fkt_cur_data
	.size	fkt_cur_data,4
fkt_cur_data:
	.zero	4

################################################################################
#
# This gets room for the probe recording.
#
# It is pretty hacky to use it indeed :), one has to use this definition of
# commas:
#define COMMA ,
#
# It changes eax, ecx and edx, and even ebx on SMP
# then returns the slot address in ecx on UP
# plus the limit in ebx and the data page address on TOS on SMP
#
# the leal_ecx_eax argument must do something like leal room(%ecx),%eax. It might use
# ebx or edx.
#
# leal_eax_ebx is something like leal room(%eax), %ebx and might use ecx
#
# inc_bl is something like addl room,%bl and may not use any register, sorry :)
# beware, limit (ebx) is pushed just before calling it !
#
#ifdef CONFIG_SMP
#define GET_ROOM(leal_eax_ebx,inc_bl,leal_ecx_eax) \
	movl	%edx,%ecx;		\
	movl	%eax,%ebx;		/* for (edx:eax) merely match memory's content */\
	lock				/* atomically */\
	cmpxchg8b fkt_next_slot;	/* have current position in edx:eax */\
1:					\
/* there may seem to be a race condition here: if the page is switched now */\
/* (just after reading) and sendfile starts to write it; but since we will */\
/* detect the page switch, we won't actually write anything in this page */\
					\
	lock				/* increment this data page counter to prevent */\
	incl	4(%edx);		/* sendfile from writing it before we're done */\
					\
	leal_eax_ebx;			/* add room to eax, getting new position in ebx */\
					\
	movl	%eax,%ecx;		\
	decl	%ecx;			/* to get on the right side */\
	andl	$PAGE_MASK,%ecx;	/* ecx gets ptr to the beginning of page */\
	negl	%ecx;			/* ecx = ebx - ecx */\
	addl	%ebx,%ecx;		/* ecx gets offset from first slot of page */\
					\
	cmpl	$PAGE_SIZE,%ecx;	/* are we off the end of the buffer? */\
	ja	4f;			/* offset too big, try to switch page */\
					\
/* room without switching page */	\
	movl	%edx,%ecx;		/* same page, but ebx was increased */\
	lock				\
	cmpxchg8b fkt_next_slot;	/* atomicaly try to get this room (ecx:ebx) */\
	jz	2f;			/* got it! */\
	lock				\
	decl	4(%ecx);		/* free the page we tried */\
	jmp	1b;			/* and redo from start */\
2:					\
	pushl	%ecx;			/* push the page entry, for final decrementing */\
	movl	%eax,%ecx;		/* ecx gets ptr to first slot for this record */\
	jmp	3f;			\
					\
4:					\
/* compute next data page address */\
	movl	%edx,%ecx;		\
	movl	%edx,%ebx;		\
	andl	$(~(FKTPTRSSIZE-1)),%ebx;	/* get base address */\
	addl	$STRUCT_DATA_PAGE_SIZE,%ecx;	/* jump to the next data page */\
	andl	$(FKTPTRSSIZE-1),%ecx;	/* mask hi bits out to wrap within the buffer */\
	orl	%ebx,%ecx;		/* mix with base address */\
					\
	movl	(%ecx),%ebx;		/* get address of next page's address, */\
	testl	%ebx,%ebx;		/* offset 0 in struct fkt_data_page */\
	jnz	4f;			/* if zero, no page left */\
	lock				\
	decl	4(%edx);		/* unlock page */\
	jmp	7f;			/* set fkt_active to zero and go out */\
4:					\
	movl	(%ebx),%ebx;		/* else get next page's address */\
	addl	$4,%ebx;		/* take room for the size */\
	pushl	%ebx;			/* remember our slot address */\
	inc_bl;				/* take room for the slot */\
	pushl	%edx;			/* and the page we locked */\
	lock				\
	cmpxchg8b fkt_next_slot;	/* atomicaly try to get this room (ecx:ebx) */\
	jz	5f;			/* got it! */\
	popl	%edx;			/* unlock the page we locked */\
	lock				/* at the beginning */\
	decl	4(%edx);		\
	popl	%ebx;			/* didn't get it, forget the slot */\
	jmp	1b;			/* and redo from start */\
					\
5:					\
/* switch succeeded, finalize it */	\
/* old next_slot is still in %eax */\
/* first write the page's fill size at its beginnig */\
	popl	%edx;			/* forget the old page */\
	pushl	%ebx;			/* save limit */\
	movl	%eax,%ebx;		/* save for size computation */\
	decl	%ebx;			/* to get on the right side */\
	andl	$PAGE_MASK,%ebx;	/* get base address of page */\
	subl	%ebx,%eax;		/* get size */\
	movl	%eax,(%ebx);		/* and write it down */\
					\
/* this precedence is used just in case another switch already happened ;) */\
	lock				\
	incl	4(%ecx);		/* protect this new page */\
	lock				\
	decl	4(%edx);		/* and let sendfile write the previous one */\
/* wake it up for execution on cpu0 XXX should try CURRENT->cpu instead */	\
	lock				\
	orl	$(1<<FKT_SOFTIRQ),irq_stat; \
					\
	popl	%ebx;			/* get limit back */\
	xchgl	%ecx,(%esp);		/* put page entry for final decrementing & get our slot address */\
3:
#else

###############################################################################
#
# UP version
#


#define GET_ROOM(leal_eax_ebx,inc_bl,leal_ecx_eax) \
	movl	fkt_next_slot,%ecx;	/* ecx gets ptr to first slot for this record */\
	leal_ecx_eax;			/* add room to ecx -> eax */\
	movl	%ecx,%edx;		\
	decl	%edx;			/* to get on the right side */\
	andl	$PAGE_MASK,%edx;	/* edx gets ptr to the beginning of the page */\
	negl	%edx;			/* edx = eax - edx */\
	addl	%eax,%edx;		/* edx gets offset from first slot of page */\
	cmpl	$PAGE_SIZE,%edx;	/* are we off the end of the buffer? */\
	ja	4f;			/* too big, try to switch page */\
	movl	%eax,fkt_next_slot;	/* update ptr to next slot after record */\
	jmp	3f;			\
4:					\
/* fkt_next_slot is still in %ecx */	\
/* first wake fkt up */			\
	orl	$(1<<FKT_SOFTIRQ),irq_stat;	/* +0 : offset of __softirq_pending in irq_cpustat_t */	\
\
/* write the page's fill size at its beginning */	\
	movl	%ecx,%edx;		/* save for size computation */\
	decl	%ecx;			/* to get on the right side */\
	andl	$PAGE_MASK,%ecx;	/* get base address of page */\
	subl	%ecx,%edx;		/* get size */\
	movl	%edx,(%ecx);		/* and write it down */\
					\
	movl	fkt_cur_data,%ecx;	/* get address of address of current page's address */\
	movl	%ecx,%edx;		\
	andl	$(~(FKTPTRSSIZE-1)),%edx;	/* get base address */\
	addl	$STRUCT_DATA_PAGE_SIZE,%ecx;	/* jump to the next one */\
	andl	$(FKTPTRSSIZE-1),%ecx;	/* mask hi bits out to wrap within the buffer */\
	orl	%edx,%ecx;		/* mix with base address */\
					\
	movl	(%ecx),%eax;		/* get address of next page's address */\
	testl	%eax,%eax;		\
	jz	7f;			/* if zero, no page left !! */\
					\
	movl	%ecx,fkt_cur_data;	/* else write it back */\
	movl	(%eax),%ecx;		/* get next page's address */\
	addl	$4,%ecx;		/* take room for the size */\
	leal_ecx_eax;			/* add room to ecx -> eax */\
	movl	%eax,fkt_next_slot;	/* remember it */\
3:
#endif /* CONFIG_SMP */


################################################################################
#
# Common slot filling
#
# this must be called with eax containing the probe code, ecx pointing on
# the slot, and on SMP, ebx holding the address of the page
#
################################################################################

/* at most */
#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,6,0)))

#define LOAD_PID \
	movl	pid(%eax),%edx		/* eax gets pid of current process */

#ifdef CONFIG_SMP
#define LOAD_CPU \
	movl	processor(%eax),%eax	/* eax get current processor no */
#else
#define LOAD_CPU
#endif

#else

#define LOAD_PID \
	movl	TI_task(%eax),%edx;	/* %edx points on task_struct */\
	testl	%edx,%edx;		/* check for task_struct != NULL */\
	jz	4f;			/* NULL, set -1 as special pid */\
	movl	pid(%edx),%edx;		/* edx gets pid of current process */\
	jmp	5f;			/* skip -1 */\
4:					\
	decl	%edx;			/* set -1 as special pid */\
5:

#ifdef CONFIG_SMP
#define LOAD_CPU \
	movl	TI_cpu(%eax),%eax	/* eax get current processor no */
#else
#define LOAD_CPU
#endif

#endif /* at least 2.6.0 (at most) */

#ifdef CONFIG_SMP
#define WRITE_IT \
	movw	%dx,probe_pid(%ecx);	/* 2nd slot (lower half) gets pid */\
	movw	%ax,probe_cpu_no(%ecx);	/* 2nd slot (upper half) gets cpu no */\
	lock				\
	decl	4(%ebx)			/* free the page */
#else
#define WRITE_IT \
	movl	%edx,probe_pid(%ecx)	/* whole 2nd slot gets pid (top half 0) */
#endif /* CONFIG_SMP */

#define COMMON_FILL \
	movl	%eax,probe_code(%ecx);	/* save code */\
					\
	rdtsc;				/* get 64-bit cpu cycle counter in edx-eax */\
					/* ignore high-order 32 bits in edx */\
	movl	%eax,probe_cycles(%ecx);	/* low-order 32 bits from eax to 1st slot */\
					\
	GET_CURRENT(%eax);		/* point back ebx at current task control block, */\
	LOAD_PID;			/* load pid in edx */\
	LOAD_CPU;			/* load cpu in eax, if SMP */\
	WRITE_IT			/* and write this in the slot */\

	.text

################################################################################
#
# normal entry point to take a probe
#	at entry	return address is on stack pointed to by esp
#			eax, ecx, edx are undefined
#			ebx is unused
#	on return	eax, ecx, edx are destroyed
#			ebx is unchanged on UP, changed on SMP but saved to
#			match standard calling conventions
#

ENTRY(fkt_header)
	pushfl				# save processor flags
	cli				# block local interrupts
#ifdef CONFIG_SMP
	pushl	%ebx
#endif

	GET_ROOM(
	/* ebx gets no of bytes needed by this record */
	movzbl	(caller_code+4)(%esp) COMMA %ebx;
	/* then ptr to next slot after record */
	addl	%eax COMMA %ebx;
	,
	/* just increment bl, this is done after push ebx, so use _offset version */
	addb	(caller_offset+4)(%esp) COMMA %bl;
	,
	movzbl	caller_code(%esp) COMMA %eax;
	addl	%ecx COMMA %eax;
)
	rdtsc				# get 64-bit cpu cycle counter in edx-eax
					# ignore high-order 32 bits in edx
	movl	%eax,probe_cycles(%ecx)	# low-order 32 bits from eax to 1st slot
	GET_CURRENT(%eax)		# point eax at current task control block

/* at most */
#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,6,0)))
	movl	pid(%eax),%edx		# edx gets pid of current process
#ifdef CONFIG_SMP
	movl	processor(%eax),%eax	# eax get current processor no
#endif
#else
	movl	TI_task(%eax),%edx	
	movl	pid(%edx),%edx		# edx gets pid of current process
#ifdef CONFIG_SMP
	movl	TI_cpu(%eax),%eax	# eax get current processor no
#endif
#endif // at least 2.6.0 (at most)

# pid is in %edx (%dx on SMP), and on SMP, %ax holds cpu

#ifdef CONFIG_SMP
	movw	%dx,probe_pid(%ecx)	# 2nd slot (lower half) gets pid
	movw	%ax,probe_cpu_no(%ecx)	# 2nd slot (upper half) gets cpu no
	leal	(caller_offset+4)(%esp),%edx	# edx points at header value in call
#else
	movl	%edx,probe_pid(%ecx)	# whole 2nd slot gets pid (top half 0)
	leal	caller_code(%esp),%edx	# edx points at header value in call
#endif /* CONFIG_SMP */

	leal	probe_code(%ecx),%ecx	# ecx points at header slot in record
9:
	movl	(%edx),%eax		# pick up next value from caller
	movl	%eax,(%ecx)		# save that value into next record slot
	leal	4(%edx),%edx		# bump edx to point at next caller value
	leal	4(%ecx),%ecx		# bump ecx to point at next record slot

#ifdef CONFIG_SMP
	cmpl	%ecx,%ebx		# see if record filled yet
	jne	9b			# if not, jump back to next loop iteration
	popl	%ecx			# pop page entry address
	lock
	decl	4(%ecx)			# free the page
#else
	cmpl	%ecx,fkt_next_slot	# see if record filled yet
	jne	9b			# if not, jump back to next loop iteration
#endif /* CONFIG_SMP */

6:
#ifdef CONFIG_SMP
	popl	%ebx
#endif
	popfl				# restore processor flags, local interrupts
	ret				# return to caller

	.align	4
7:
	movl	$0,fkt_active		# turn all active bits off
	jmp	6b			# leave


#define GET_ROOMn(n)	\
	GET_ROOM( \
	leal	n(%eax) COMMA %ebx; \
	/* ebx gets ptr to next slot after record */ \
	, \
	addb	$n COMMA %bl; \
	, \
	leal	n(%ecx) COMMA %eax; \
)

################################################################################
#
#	fastest entry point to take a probe for sys calls and traps (with no params)
#	probe record is 12 bytes
#	at entry	return address is on stack pointed to by esp
#			eax contains the code
#			ebx is undefined
#			ecx is undefined
#			edx is undefined
#	on return	eax, ecx, edx are destroyed
#			ebx is set to pointer to the TCB
#

ENTRY(fkt_eax_ebx0)
	pushfl				# save processor flags
	cli				# block local interrupts
	pushl	%eax			# save code

	GET_ROOMn(12)

#ifdef CONFIG_SMP
	popl	%ebx
#endif
	popl	%eax

	COMMON_FILL

6:
/* at most */
#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,6,0)))
#ifdef CONFIG_SMP
	GET_CURRENT(%ebx)		/* %ebx has been lost in GET_ROOM */
#endif /* CONFIG_SMP */
#endif
	popfl				# restore processor flags, local interrupts
	ret				# return to caller

	.align	4
7:
	movl	$0,fkt_active		# turn all active bits off
	popl	%eax			# to the pop of label 3
	jmp	6b			# leave


################################################################################
#
#	faster entry point to take a probe for IRQs and other probes with no params
#	probe record is 12 bytes
#	at entry	return address is on stack pointed to by esp
#			eax contains the code
#			ebx is unused
#			ecx is undefined
#			edx is undefined
#	on return	eax, ecx, edx are destroyed
#			ebx is unchanged on UP, changed on SMP, don't call this
#			as a regular function !!
#

ENTRY(fkt_eax0)
	pushfl				# save processor flags
	cli				# block local interrupts
	pushl	%eax

	GET_ROOMn(12)

#ifdef CONFIG_SMP
	popl	%ebx			# pop the data page
#endif
	popl	%eax

	COMMON_FILL
6:
	popfl				# restore processor flags, local interrupts
	ret				# return to caller

	.align	4
7:
	movl	$0,fkt_active		# turn all active bits off
	popl	%eax			# to the pop of label 3
	jmp	6b			# leave


################################################################################
#
#	faster entry point to take a probe with one parameter
#	expects 1 parameter in %edx, and code in %eax
#	probe record is 16 bytes
#	at entry	return address is on stack pointed to by esp
#			eax contains the code
#			ebx is unused
#			ecx is undefined
#			edx contains the parameter
#	on return	eax, ecx, edx are destroyed
#			ebx is unchanged on UP, changed on SMP, don't call this
#			as a regular function !!
#

ENTRY(fkt_eax1)
	pushfl				# save processor flags
	cli				# block local interrupts
	pushl	%eax			# save the code because we need eax
	pushl	%edx			# save the parameter because we need edx

	GET_ROOMn(16)

#ifdef CONFIG_SMP
	popl	%ebx			# pop the data page
#endif
	popl	%edx			# get back parameter
	movl	%edx,probe_param1(%ecx)	# save first (only) parameter
	popl	%eax			# get back code

	COMMON_FILL
6:
	popfl				# restore processor flags, local interrupts
	ret				# return to caller

	.align	4
7:
	movl	$0,fkt_active		# turn all active bits off
	popl	%edx			# get back parameter
	popl	%eax			# get back code
	jmp	6b			# leave

################################################################################
#
#	entry points for automatic function instrumenting:
#
#	these are automagically called on entry and exit of every fonction of
#	every file compiled with the -finstrument-fuctions, except functions
#	with __attribute__ ((no_instrument_function))
#
#	at entry	this_fn at 8(%esp), call_site at 4(%esp)
#	at return	eax, ecx, edx are destroyed
#			ebx is kept

ENTRY(__cyg_profile_func_enter)
	testl	$FKT_GCC_INSTRUMENT_KEYMASK,fkt_active
	jnz	0f
	ret
0:
	pushfl				# save processor flags
	cli				# block local interrupts
	pushl	%ebx			# save ebx

	GET_ROOMn(16)

#ifdef SMP
	popl	%ebx			# pop the data page
#endif
	movl	12(%esp),%edx
	movl	%edx,probe_param1(%ecx)	# save first (only) parameter
	movl	$(((FKT_GCC_INSTRUMENT_ENTRY_CODE)<<8)|16),%eax	# code

	COMMON_FILL
6:
	popl	%ebx			# restore ebx
	popfl				# restore processor flags, local interrupts
	ret				# return to caller

	.align	4
7:
	movl	$0,fkt_active		# turn all active bits off
	jmp	6b

ENTRY(__cyg_profile_func_exit)
	testl	$FKT_GCC_INSTRUMENT_KEYMASK,fkt_active
	jnz	0f
	ret
0:
	pushfl				# save processor flags
	cli				# block local interrupts
	pushl	%ebx			# save ebx

	GET_ROOMn(16)

#ifdef SMP
	popl	%ebx			# pop the data page
#endif
	movl	12(%esp),%edx
	movl	%edx,probe_param1(%ecx)	# save first (only) parameter
	movl	$(((FKT_GCC_INSTRUMENT_EXIT_CODE)<<8)|16),%eax	# code

	COMMON_FILL
6:
	popl	%ebx			# restore ebx
	popfl				# restore processor flags, local interrupts
	ret				# return to caller

	.align	4
7:
	movl	$0,fkt_active		# turn all active bits off
	jmp	6b

################################################################################
#
	.globl	fkt_trailer		# to see size of code generated
fkt_trailer:
	.size   fkt_header,fkt_trailer-fkt_header
