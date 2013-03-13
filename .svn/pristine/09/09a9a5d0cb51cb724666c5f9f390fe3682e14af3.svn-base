/*
 * fkt.c
 *
 *  Copyright (C) 2003, 2004 Samuel Thibault <samuel.thibault@ens-lyon.org>
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

/*	fkt = Fast Kernel Tracing */

/* these are the always-resident data & functions */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/mmzone.h>
#include <linux/swap.h>
#include <linux/pagemap.h>
#include <linux/interrupt.h>
#include <asm/current.h>
#include <asm/errno.h>
#include <linux/fkt.h>

#if defined(CONFIG_FKT) || defined(CONFIG_FKT_MODULE)
/* set to non-zero when probing is active */
volatile unsigned int fkt_active;

/* so that probes can wake the recording process up through the softirq */
struct task_struct *fkt_sendfile_proc;

#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,5,56)))
/* these versions don't have a sendfile operation, so sys_sendfile needs to
 * call this explicitely */
ssize_t (*fkt_sendfile)(struct file *, loff_t *, size_t, read_actor_t, void *);
EXPORT_SYMBOL(fkt_sendfile);
#endif

EXPORT_SYMBOL(fkt_active);
EXPORT_SYMBOL(fkt_sendfile_proc);

/* defined in fkt_header.S */
EXPORT_SYMBOL(fkt_cur_data);
EXPORT_SYMBOL(fkt_next_slot);
EXPORT_SYMBOL(fkt_header);
EXPORT_SYMBOL(fkt_eax0);
EXPORT_SYMBOL(fkt_eax1);
EXPORT_SYMBOL(__cyg_profile_func_enter);
EXPORT_SYMBOL(__cyg_profile_func_exit);
#endif

/* softirq action: wake the recording process up */
static void fkt_sendfile_wake(struct softirq_action *foo)
{
	struct task_struct *proc=fkt_sendfile_proc;
	if (proc)
		wake_up_process(proc);
}

/* setup this softirq on boot */
static __init int setup_softirq(void)
{
	open_softirq(FKT_SOFTIRQ, fkt_sendfile_wake, NULL);
	return 0;
}

__initcall(setup_softirq);
