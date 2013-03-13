/*
 * fkt-mod.c
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

/* these are the modular data & functions */

/* not tested on 2.5 & 2.6 */

#include <linux/config.h>

//#define DEBUG 1
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <linux/notifier.h>
#include <linux/list.h>
#include <linux/pagemap.h>
#include <asm/semaphore.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/page.h>
#if (LINUX_VERSION_CODE >= (KERNEL_VERSION(2,5,28)))  // at most
#include <linux/buffer_head.h>
#else
#define page_has_buffers(page) (!!((page)->buffers))
#define page_buffers(page) ((page)->buffers)
#endif

#include <linux/fkt.h>

/*
 * parameters that can be set via an ioctl
 */

/* initial value of fkt_active */
static unsigned int initmask;
static int initpowpages = FKTINITPOWPAGES;
static unsigned trydma;			/* only useful for ISA disk boards */

/* current number of allocated pages */
static int nbpages;

/*
 * exclusive fkt_sendfile access
 */
/* held by the unique recording process */
static DECLARE_MUTEX(fkt_sem);
/* list of processes waiting for fkt to be ready */
static DECLARE_WAIT_QUEUE_HEAD(ready_queue);

/*
 * this holds nbpages addresses of pages
 */
static unsigned long *pagebuf;

/*
 * arithmetic of page pointers
 */
#define FKTMAXNBPAGES	(PAGE_SIZE/sizeof(unsigned long))
#define PAGESUCC(p) ((typeof(p)) ( ((unsigned long) pagebuf) | \
		((((unsigned long)p) % (FKTMAXNBPAGES*sizeof(unsigned long))+sizeof(unsigned long)) % (nbpages*sizeof(unsigned long)))))
#define PAGEPRED(p) ((typeof(p)) ( ((unsigned long) pagebuf) | \
		((((unsigned long)p) % (FKTMAXNBPAGES*sizeof(unsigned long))+(nbpages-1)*sizeof(unsigned long)) % (nbpages*sizeof(unsigned long)))))
#define PAGEINC(p) (p = PAGESUCC(p))
#define PAGEDEC(p) (p = PAGEPRED(p))

/*
 * rotating buffer scheme :
 */

/* this holds pointers to addresses of pages */
static struct fkt_data_page *rotatingbuf;

/*
 * a free page has counter to 1
 * when it is queued for probes, it gets to 2
 * it is checked in, which gets it (hence counter to 3), locks it, reserves it,
 * marks it as updated but dirty.
 *  generic_file_write gets it (hence counter to 4) and locks it
 *   prepare_write may then to block_prepare_write, ie may create buffer heads
 *   and get it once more (hence counter to 5)
 *   commit_write dirties this buffers, if any, or gets the page to send it
 *   (hence counter always to 5)
 *  generic_file_write unlocks the page, puts it, so that the counter gets to 4
 *
 * when page send is completed, the page is put back (ie counter to 3)
 * if buffers are written, they are free, so that they may get rid of, and the
 * page put back (ie counter to 3 as well)
 *
 * the counter of a page can have either of those values (X can be any value
 * greater than 1)
 * 0: shouldn't happen until a call to free_buffer()
 * 1: it is allocated, and free for queuing
 * 2: it is queued, and being filled up if not filled up yet
 * 3: it is free for queuing !
 * XRD: it is in the page cache, but still dirty (writes still pending)
 * XR: it is in the page cache, and up to date, it can be checked out
 *
 * hence, the buffer looks like
 *
 *                               towrite_data fkt_cur_data limit_data
 *                               v            v            v
 *           DD  D  DDDDDDDDDDDDD
 *         RRRR RR RRRRRRRRRRRRRR
 * 11111111XXXX1XX3XXXXXXXXXXXXXX22222222222222222222222222111111
 * free    written   writing     filled       filling      free
 *             ^ some may already be freed
 *                         ----->
 */

static struct fkt_data_page *towrite_data;
	/* data to check in to the cache */
extern struct fkt_data_page * volatile fkt_cur_data;	/* current data page, VOLATILE */
	/* data filling up */
static struct fkt_data_page *limit_data;;
	/* at least one NULL */

/*
 * arithmetic of rotating pointers
 */
#define ROTSUCC(p) ((typeof(p)) ( ((unsigned long) rotatingbuf) | \
		(((unsigned long)(p+1)) % FKTPTRSSIZE)))
#define ROTPRED(p) ((typeof(p)) ( ((unsigned long) rotatingbuf) | \
		(((unsigned long)(p-1)) % FKTPTRSSIZE)))
#define ROTINC(p) (p = ROTSUCC(p))
/* p is assumed to be greater than q */
#define ROTDIFF(p,q) (((p+FKTMAXNBPTRS)-q)%FKTMAXNBPTRS)

/* how to enqueue a page */
#ifdef DEBUG
#define POISON(a) \
memset((void *)*a,FKT_POISON&0xff,PAGE_SIZE)	/* poisoning just for debugging */
#else
#define POISON(a)
#endif

#define ENQUEUE(a) {\
	pr_debug("fkt: queuing page %d at %d\n",a-pagebuf,limit_data-rotatingbuf);\
	POISON(a);\
	get_page(virt_to_page(*a));\
	limit_data->page=a;\
	ROTINC(limit_data);\
}

/*
 * allocate pages with some checks
 */
static inline struct page *fkt_alloc_pages(unsigned int gfp_mask, unsigned int order) {
	struct page *p = alloc_pages(gfp_mask,order);
	int i;

	if (!p) return p;
	for (i=0;i<1<<order;i++) {
		set_page_count(p+i,1);
		set_bit(PG_FKT,&(p+i)->flags);
		BUG_ON(page_has_buffers(p+i));
		BUG_ON((p+i)->mapping);
		//(p+i)->list.next = (struct list_head *) 0xf4375970; // poisoning
		//(p+i)->list.prev = (struct list_head *) 0xf4375971;
	}
	return p;
}

/*
 * alloc / free probes record buffers.
 * these 2 must be called with fkt_sem held
 */
static void free_buffer(unsigned long *freepagebuf, int freepages)
{
	unsigned long *a;
	struct page *p;

	if (!freepages) return;
	printk(KERN_DEBUG "fkt: freeing buffer\n");

	for (a = freepagebuf; nbpages--, freepages--; a++) {
		p = virt_to_page(*a);
		if (page_has_buffers(p))
			PAGE_BUG(p);
		if (p->mapping)
			PAGE_BUG(p);
		clear_bit(PG_FKT,&p->flags);
		if (page_count(p)!=1) {
			static int oops;
			if (!oops++)
				printk(KERN_WARNING "fkt: page count is not 1 on freeing\n");
			set_page_count(p,1);
		}
		free_page(*a);
		*a = 0;
	}
}

static int alloc_buffer(unsigned long *allocpagebuf, int powpages)
{
	struct page *p;
	unsigned long *a;
	int ret,i;

	/* we try to get pages in dma zone whenever possible
	 * (only useful for ISA controlers) */
	p = fkt_alloc_pages((GFP_KERNEL|trydma)&(~__GFP_WAIT),powpages);
	if (trydma && !p)
		p = fkt_alloc_pages(GFP_KERNEL&(~__GFP_WAIT),powpages);
	/* if couldn't get it as a whole bunch, split it up */
	if (!p) {
		if (!powpages)
			return -ENOMEM;	/* can't cut in more little pieces */
		/* try with smaller bunches */
		if ((ret = alloc_buffer(allocpagebuf,powpages-1))<0)
			return ret;
		if ((ret = alloc_buffer(allocpagebuf+(1<<(powpages-1)),powpages-1))<0)
			free_buffer(allocpagebuf,1<<(powpages-1));
		return ret;
	}

	printk(KERN_DEBUG "fkt: allocated buffer with pow %d at 0x%p\n",powpages,page_address(p));

	for (a = allocpagebuf, i=0; i < 1<<powpages; a++, i++, nbpages++)
		*a = (unsigned long) page_address(p+i);

	return 0;
}

/*
 * synchronizing thread: spawned at module insertion,
 * write a struct file * in fsync_file, send wake it up to have your file
 * asynchronously flushed.
 */
static struct file *fsync_file;
static struct task_struct *fsync_task;
static DECLARE_MUTEX_LOCKED(fsync_ended);
static int fsync_thread(void *startup)
{
	int ret;
	static int i=0;
	struct address_space *mapping;
	/* taken from drivers/scsi/scsi_error.c */
#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,6,4)))
#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,6,0)))  // at most
	daemonize();
	sprintf(current->comm, "fkt_flush");
#else
	daemonize("fkt_flush");
#endif
#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,5,38)))  // at most
	reparent_to_init();
#endif
#endif
	fsync_task=current;
	wmb();
	complete((struct completion *)startup);

	while (1) {
		if (fsync_file) {
			pr_debug("fkt: not much space, forcing sync\n");
			FKT_PROBE0(FKT_FKT_KEYMASK,FKT_FKT_FORCE_SYNC_ENTRY_CODE);
			mapping = fsync_file->f_mapping;
			if ((ret = filemap_fdatawrite(mapping)) && i++<10)
				printk("filemap_fdatawrite returned %d\n",ret);
			// this may wait within some fs...
			if ((ret = fsync_file->f_op->fsync(fsync_file,fsync_file->f_dentry,0))<0 && i++<10)
				printk("fsync returned %d\n",ret);
			// no need to really wait...
			/*
			if ((ret = filemap_fdatawait(mapping)) && i++<10)
				printk("filemap_fdatawait returned %d\n",ret);
			*/
			FKT_PROBE1(FKT_FKT_KEYMASK,FKT_FKT_FORCE_SYNC_EXIT_CODE,ret);
		}
		fsync_file = NULL;
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
		if (signal_pending(current))
			break;
	}
	fsync_task=NULL;
	up(&fsync_ended);
	return 0;
}

/*
 * our replacement for a future generic sendpage function
 */
static ssize_t sendpage(struct file *out_file, read_descriptor_t *desc,
		struct page *p, unsigned long size,
		int actor(read_descriptor_t *desc, struct page *page,
			unsigned long offset, unsigned long size)) {
	unsigned long index = out_file->f_pos >> PAGE_CACHE_SHIFT;
	int ret;

	if (!out_file->f_op->sendpage) {
	/* if the file does not support sendpage, we first have to put it in
	 * the page cache to avoid unnecessary copy */
		if (page_has_buffers(p) || p->mapping)
			PAGE_BUG(p);
		if (!(ret = checkin_cache_page(p, out_file->f_dentry->d_inode->i_mapping, index, GFP_KERNEL))) {
		/* unlock it since it's already ready :) */
			unlock_page(p);
		} else {
			if (ret == -EEXIST)
				printk(KERN_WARNING "fkt: page cache already filled, please don't use output file while tracing.\n");
			else
				printk(KERN_WARNING "fkt: checkin failed !: %d\n",ret);
		}
	}
	/* and send the page */
	return actor(desc, p, 0, size);
}

/*
 * the FKT sendfile method, heart of the recording process
 */
static ssize_t sendfile(struct file *in_file, loff_t *ppos, size_t count, read_actor_t actor, void __user *target)
{
	read_descriptor_t desc;
	int powpages=initpowpages;
	int grows=powpages;
	struct file *out_file = (struct file *) target;

	/* the three levels of pointers on a page */
	//struct fkt_data_page *d;	/* a pointer in the rotating buffer, unused :) */
	unsigned long *a;		/* a pointer in the page buffer */
	struct page *p;			/* a pointer to a page */

	ssize_t retval = PAGE_SIZE;	/* usual return of actor, 0 means error */
	int ret;

	if (!count)
		return 0;

	if (out_file->f_pos & (PAGE_SIZE-1UL))
		/* refuse non-aligned writes */
		return -EINVAL;

#ifdef DEBUG
#ifdef CONFIG_SMP
	/* check that cmpxchg8b will find its variables */
	if (((void *) &fkt_cur_data) - ((void *) &fkt_next_slot) != 4) {
		printk(KERN_ERR "fkt_next_slot and fkt_cur_data are not grouped, hence it can't run on SMP.\n"
			KERN_ERR "Please edit arch/i386/kernel/fkt_header.S\n");
		return -ENOSYS;
	}
#endif
#endif

	if (in_file->f_flags & O_NONBLOCK) {
		if (down_trylock(&fkt_sem))
			return -EBUSY;
	} else {
		if (down_interruptible(&fkt_sem))
			return -ERESTARTSYS;
	}

	fkt_active = 0;			/* just to be sure */

	/* allocate the rotating buffer */
	if (!rotatingbuf) {
		/* keep that allocation in that order */
		rotatingbuf = (struct fkt_data_page *) get_zeroed_page(GFP_KERNEL);
		if (!rotatingbuf) {
			ret = -ENOMEM;
			goto out_sem;
		}
		printk(KERN_DEBUG "fkt: allocated data pointers page at %p\n",rotatingbuf);
		pagebuf = (unsigned long *) get_zeroed_page(GFP_KERNEL);
		if (!pagebuf) {
			free_page((unsigned long) rotatingbuf);
			rotatingbuf = NULL;
			ret = -ENOMEM;
			goto out_sem;
		}
		printk(KERN_DEBUG "fkt: allocated pointers page at %p\n",pagebuf);
	}

	/* allocate pages if not done already or not enough */
	if (nbpages < 1<<powpages) {
		free_buffer(pagebuf,nbpages);
		ret = alloc_buffer(pagebuf,powpages);
		if (ret) goto out_sem;
	}

	/* initialize counters and pointers */
	printk("fkt: started, count=%dMo\n",count>>20);

	desc.written = 0;
	desc.count = count;
	desc.arg.buf = target;
	desc.error = 0;

	towrite_data =
	fkt_cur_data =
	limit_data =
	rotatingbuf;

	/* trivial initial mapping */
	for (a=pagebuf;limit_data!=ROTPRED(towrite_data) && a<pagebuf+nbpages;a++) {
		atomic_set(&limit_data->count,0);
		ENQUEUE(a);
	}
	memset(limit_data,0,(FKTMAXNBPTRS-(limit_data-rotatingbuf))*sizeof(struct fkt_data_page));

	/* give addresses to probes */
	fkt_next_slot = ((unsigned *) *(fkt_cur_data->page)) + 1; /* +1 for page fill size */

	/* and launch them */
	fkt_active = initmask;
	printk("fkt: active mask 0x%08x\n", fkt_active);

	/* do the initial probes */
	FKT_PROBE4(-1, FKT_SETUP_CODE, initmask, jiffies, count, nbpages);
	FKT_PROBE0(-1, FKT_CALIBRATE0_CODE);
	FKT_PROBE0(-1, FKT_CALIBRATE0_CODE);
	FKT_PROBE0(-1, FKT_CALIBRATE0_CODE);
	FKT_PROBE1(-1, FKT_CALIBRATE1_CODE, 1);
	FKT_PROBE1(-1, FKT_CALIBRATE1_CODE, 1);
	FKT_PROBE1(-1, FKT_CALIBRATE1_CODE, 1);
	FKT_PROBE2(-1, FKT_CALIBRATE2_CODE, 1, 2);
	FKT_PROBE2(-1, FKT_CALIBRATE2_CODE, 1, 2);
	FKT_PROBE2(-1, FKT_CALIBRATE2_CODE, 1, 2);

	/* now let people go */
	fkt_sendfile_proc = current;
	wake_up_all(&ready_queue);

	/*
	 * and enter the big loop
	 */
	do {
		static int stopped = 0;
		if (unlikely(!fkt_active && !stopped++)) {
			/* fkt_active is 0 ? Hum. Either initmask was already
			 * 0, or probes went to the end of the rotating buffer,
			 * it can be set back with fkt_setmask anyway, so
			 * just warn */
			printk("fkt: fkt_active dropped down to 0%s\n",
						fkt_cur_data==limit_data?", probably because there is no room anymore":"");
		}

		set_current_state(TASK_INTERRUPTIBLE);
		schedule();

		/* if there is less than a page to be written, well since
		 * we've been waked up, we have it already */
		if (desc.count<PAGE_SIZE)
			break;

		if (unlikely(signal_pending(current))) {
			printk(KERN_DEBUG "fkt: killed\n");
			FKT_PROBE0(-1,FKT_RECORD_KILLED_CODE);
			break;
		}

		/* woken up, do our job */

		/* launch write orders if needed */
		for (; towrite_data!=fkt_cur_data && desc.count
#ifdef CONFIG_SMP
				/* wait for pending probes */
				&& atomic_read(&towrite_data->count)==0
#endif
				; ROTINC(towrite_data)) {
			pr_debug("fkt: writing page %d\n",towrite_data->page-pagebuf);
			p = virt_to_page(*towrite_data->page);
			FKT_PROBE1(FKT_FKT_KEYMASK,FKT_RECORD_WRITING_CODE,towrite_data->page-pagebuf);
			towrite_data->page = NULL;
			if ((retval = sendpage(out_file,&desc,p,PAGE_SIZE,actor)) != PAGE_SIZE)
				goto out;
		}

		/* now feed the probes */
		/* first the sequential feeding, which doesn't fragment */
		for (a=(PAGESUCC((ROTPRED(limit_data))->page));
				limit_data!=ROTPRED(towrite_data);
				PAGEINC(a)) {
			p = virt_to_page(*a);
			if (p->mapping) {/* if checked in, try to check out */
				if (test_and_set_bit(PG_locked,&p->flags))
					/* still locked */
					break;
				/* page locked, we can have a look at its counter & co */
				if (page_count(p)>3 &&
					!(page_has_buffers(p) &&
					  !buffers_dirty(p))) {
					clear_bit(PG_locked,&p->flags);
					break;
				}
				clear_bit(PG_locked,&p->flags);
				/* page_count is 3 or has written buffers */
				checkout_cache_page(p);
				put_page(p);
				if (page_count(p)!=1) {
					static int tried;
					if (tried++<10)
						printk(KERN_ERR "page %d (flags %lx) count is %d !\n", a-pagebuf, p->flags, page_count(p));
					set_page_count(p,1);
				}
			} else if (page_count(p)>1) /* already queued :( */
				break;
			if (page_has_buffers(p)) {
				printk(KERN_ERR "page %d (flags %lx) still has buffers @%p\n", a-pagebuf, p->flags, page_buffers(p));
				PAGE_BUG(p);
				goto out;
			}
			FKT_PROBE0(FKT_FKT_KEYMASK,FKT_RECORD_WRITTEN_CODE);
			ENQUEUE(a);
		}

		/* if low on space, force syncing */
		/* may be tuned */
		if (ROTDIFF(limit_data,fkt_cur_data) < (nbpages * 3) / 4) {
			fsync_file=out_file;
			wmb();
			wake_up_process(fsync_task);
		}

		/* then, if really needed, the general feeding, which might
		 * fragment or even allocate pages */
		/* may be tuned */
		if (ROTDIFF(limit_data,fkt_cur_data) < nbpages / 8) {
			if (nbpages>=1<<grows)
				printk(KERN_NOTICE "fkt: you should grow pages at least to power %d\n",++grows);
			if (unlikely(limit_data == ROTPRED(towrite_data))) {
				static int oops;
				if (!oops++)
					printk(KERN_WARNING "fkt: oops, buffer grew to maximum %luM, hoping it will fit...\n",FKTMAXNBPTRS/((1<<20)/PAGE_SIZE));
			} else {
				/* probes coming to the end of buffer, hurrying */
				/* start just before and go back */
				for (a=PAGEPRED(ROTPRED(limit_data)->page);
					a!=ROTPRED(limit_data)->page; PAGEDEC(a)) {
					p = virt_to_page(*a);
					if (p->mapping) {/* if checked in, try to check out */
						if (test_and_set_bit(PG_locked,&p->flags))
							continue;
						/* page locked, we can have a look at its counter & co */
						if (page_count(p)>3 &&
							!(page_has_buffers(p) &&
							  !buffers_dirty(p))) {
							clear_bit(PG_locked,&p->flags);
							continue;
						}
						clear_bit(PG_locked,&p->flags);
						/* page_count is 3 or has written buffers (which gets_page()) */
						checkout_cache_page(p);
						put_page(p);
						if (page_count(p)!=1) {
							static int tried;
							if (tried++<10)
								printk(KERN_ERR "page count is %d !\n",page_count(p));
							set_page_count(p,1);
						}
					} else if (page_count(p)>1) /* already queued :( */
						continue;

					if (page_has_buffers(p)) {
						printk(KERN_ERR "page %d (flags %lx) still has buffers @%p\n", a-pagebuf, p->flags, page_buffers(p));
						PAGE_BUG(p);
					}
					FKT_PROBE0(FKT_FKT_KEYMASK,FKT_RECORD_EXTRA_WRITTEN_CODE);
					ENQUEUE(a);
					if (ROTDIFF(limit_data,fkt_cur_data) >= nbpages / 4 || limit_data == ROTPRED(towrite_data))
						/* that's enough for now */
						break;
				}
				if (a==ROTPRED(limit_data)->page) {
					a=pagebuf+nbpages;
					/* hum, no free page... */
					FKT_PROBE0(FKT_FKT_KEYMASK,FKT_FKT_EXTRA_ALLOC_ENTRY_CODE);
					if (nbpages==FKTMAXNBPAGES) 
					{
						static int oops;
						if (!oops++)
							printk("fkt: oops, allocated maximum %luM pages, hoping it will fit...\n",nbpages/((1<<20)/PAGE_SIZE));
					} else while (ROTDIFF(limit_data,fkt_cur_data) < nbpages / 4 && limit_data != ROTPRED(towrite_data) && nbpages<FKTMAXNBPAGES) {
						static int oops;
						*a = (unsigned long) page_address(fkt_alloc_pages((GFP_KERNEL|trydma)&(~__GFP_WAIT),0));
						if (trydma && !*a)
							*a = (unsigned long) page_address(fkt_alloc_pages(GFP_KERNEL&(~__GFP_WAIT),0));
						if (!*a) {
							if (!oops++)
								printk(KERN_WARNING "fkt: oops, couldn't get an extra page, hoping it will fit...\n");
							break;
						} else {
							nbpages++;
							FKT_PROBE0(FKT_FKT_KEYMASK,FKT_RECORD_EXTRA_PAGE_CODE);
							ENQUEUE(a);
							a++;
						}
					}
					FKT_PROBE0(FKT_FKT_KEYMASK,FKT_FKT_EXTRA_ALLOC_EXIT_CODE);
				}
			}
		}
	} while (desc.count);
out:

	/* finish up things */

	/* stop new probes */
	fkt_active = 0;
	fkt_sendfile_proc = NULL;
	printk("fkt: ok, finished, cleaning up things\n");

	/* on signal, write pending pages, including the last, whose size
	 * should be set */
	if (retval) {
		for (; desc.count && towrite_data != fkt_cur_data; ROTINC(towrite_data)) {
#ifdef CONFIG_SMP
/* wait for probes, it shouldn't last too long */
			while (atomic_read(&towrite_data->count)>0);
#endif
			pr_debug("fkt: finishing: writing page %d\n",towrite_data->page-pagebuf);
			p = virt_to_page(*towrite_data->page);
			retval = sendpage(out_file,&desc,p,PAGE_SIZE,actor);
			towrite_data->page = NULL;
			put_page(p);
			if (retval != PAGE_SIZE) /* oups, error or eof */
				goto out2;
		}
		if (desc.count && *fkt_cur_data->page) {
			unsigned long size;
			/* ok, we got to fkt_cur_data, fill in its size and write it */
			size=((unsigned long) fkt_next_slot) & (PAGE_SIZE-1UL);
			if (size) {
				if (size>desc.count)
					size=desc.count;
				*((unsigned *)(*fkt_cur_data->page))=size;
				p = virt_to_page(*fkt_cur_data->page);
				retval = sendpage(out_file,&desc,p,PAGE_SIZE,actor);
				towrite_data->page = NULL;
				put_page(p);
				ROTINC(towrite_data);
				if (retval != PAGE_SIZE) /* error or eof */
					goto out2;
			}
		}
		if (desc.count >= sizeof(unsigned)) {
			/* still some room, write end of file, but we need
			 * a brand new page for this :/ */
			p = alloc_page(GFP_KERNEL);
			if (!p) {
				desc.written = 0;
				desc.error = -ENOMEM;
			} else {
				*(unsigned *)page_address(p)=FKT_EOF;
				actor(&desc,p,0,sizeof(unsigned));
				__free_page(p);
			}
		}
	}
out2:

	fkt_next_slot = NULL;
	fkt_cur_data = NULL;

	/* synchronize out file to be able to free our pages up */
	out_file->f_op->fsync(out_file,out_file->f_dentry,1);

	/* clean rotating buffer up */
	for (; towrite_data!=limit_data; ROTINC(towrite_data)) {
		a=towrite_data->page;
#ifdef CONFIG_SMP
/* wait for probes, it shouldn't last very long */
		while (atomic_read(&towrite_data->count)>0);
#endif
		pr_debug("fkt: freeing page %d\n",a-pagebuf);
		put_page(virt_to_page(*a));
		towrite_data->page = NULL;
	}

	for (a=pagebuf;a<pagebuf+nbpages;a++) {
		p=virt_to_page(*a);
		if (p->mapping) { /* didn't checkout this one yet */
			pr_debug("fkt: checking out page %d\n",a-pagebuf);
			checkout_cache_page(p);
			set_page_count(p,1);
		}
		if (page_has_buffers(p)) {
			printk(KERN_ERR "fkt: page %d still has buffers @%p\n", a-pagebuf,page_buffers(p));
			PAGE_BUG(p);
			break;
		}
	}

	if (desc.written)
		ret = desc.written;
	else
		ret = desc.error;

out_sem:
	up(&fkt_sem);
	return ret;
}

/*
 * opening /dev/fkt
 */

static int fkt_open(struct inode *inode, struct file *filp)
{
#if (LINUX_VERSION_CODE >= (KERNEL_VERSION(2,5,30)))
	/* linux 2.5.30 gave us the great sendfile method */
	filp->f_op->sendfile = sendfile;
#endif
	return 0;
}

/*
 * ioctl on /dev/fkt
 */

static int fkt_ioctl(struct inode *inode, struct file *filp, unsigned cmd, unsigned long param)
{
	int ret = 0;
	unsigned int old_active = fkt_active;

	switch (cmd) {
	case FKT_SETINITPOWPAGES:
		if (1<<param > FKTMAXNBPAGES)
			ret = -EINVAL;
		else
			initpowpages=param;
		break;
	case FKT_SETINITMASK:
		initmask = param & FKT_KEYMASKALL;
		break;
	case FKT_SETTRYDMA:
		trydma = param ? __GFP_DMA : 0;
		break;
	case FKT_FREEBUFFER:
		if (filp->f_flags & O_NONBLOCK) {
			if (down_trylock(&fkt_sem)) {
				ret = -EBUSY;
				break;
			}
		} else {
			if (down_interruptible(&fkt_sem)) {
				ret = -ERESTARTSYS;
				break;
			}
		}
		free_buffer(pagebuf,nbpages);
		up(&fkt_sem);
		break;
	case FKT_WAITREADY:
		printk("fkt_record: waiting for fkt to be ready\n");
		wait_event(ready_queue,fkt_sendfile_proc);
		printk("fkt_record: go!\n");
		break;
	case FKT_USER_PROBE0: {
		unsigned int code;
		if (get_user(code, (u32 *)param))
			ret = -EFAULT;
		else
			fkt_header((code<<8) | 12);
		break;
	}
	case FKT_USER_PROBE1: {
		unsigned int code,p1;
		if (get_user(code, (u32 *)param)
			|| get_user(p1, ((u32 *)param) + 1))
			ret = -EFAULT;
		else
			fkt_header((code<<8) | 16, p1);
		break;
	}
	case FKT_USER_PROBE2: {
		unsigned int code,p1,p2;
		if (get_user(code, (u32 *)param)
			|| get_user(p1, ((u32 *)param) + 1)
			|| get_user(p2, ((u32 *)param) + 2))
			ret = -EFAULT;
		else
			fkt_header((code<<8) | 20, p1, p2);
		break;
	}
	default:
		goto switchmask;
	}
	return ret;

switchmask:
/*	active masks should never become negative numbers in order to prevent
	problems returning them as function results (they look like error codes!) */
	if (!fkt_next_slot) return -EPERM;
	switch (cmd) {
	case FKT_ENABLE:
		fkt_active |= param & FKT_KEYMASKALL;
		break;
	case FKT_DISABLE:
		fkt_active &= (~param) & FKT_KEYMASKALL;
		break;
	case FKT_SETMASK:
		fkt_active = param & FKT_KEYMASKALL;
		break;
	default:
		return -EINVAL;
		break;
	}
	FKT_PROBE2(-1, FKT_KEYCHANGE_CODE, fkt_active, jiffies);
	return old_active;
}

static struct block_device_operations fkt_ops = {
	.open= fkt_open,
	.ioctl= fkt_ioctl,
	.owner= THIS_MODULE,
};

/* useful when an Oops happens */
#ifdef DEBUG
static int fkt_panic_event(struct notifier_block *this,
			   unsigned long event,
			   void *ptr) {
	unsigned long *a;

	/* on panic, show the pages state */
	if (nbpages) {
		printk(KERN_EMERG "fkt: %d pages:\n", nbpages);
		printk(KERN_EMERG);
		for (a=pagebuf;a<pagebuf+nbpages;a++)
			printk(PageUptodate(virt_to_page(*a))?"U":" ");
		printk("\n");
		printk(KERN_EMERG);
		for (a=pagebuf;a<pagebuf+nbpages;a++)
			printk(PageDirty(virt_to_page(*a))?"D":" ");
		printk("\n");
		printk(KERN_EMERG);
		for (a=pagebuf;a<pagebuf+nbpages;a++)
			printk(page_has_buffers(virt_to_page(*a))?"B":" ");
		printk("\n");
		printk(KERN_EMERG);
		for (a=pagebuf;a<pagebuf+nbpages;a++)
			printk(virt_to_page(*a)->mapping?"M":" ");
		printk("\n");
		printk(KERN_EMERG);
		for (a=pagebuf;a<pagebuf+nbpages;a++)
			printk(PageReserved(virt_to_page(*a))?"R":" ");
		printk("\n");
		printk(KERN_EMERG);
		for (a=pagebuf;a<pagebuf+nbpages;a++)
			printk("%u",page_count(virt_to_page(*a)));
		printk("\n");
	}
	return NOTIFY_DONE;
}

static struct notifier_block fkt_panic_block = {
	fkt_panic_event,
	NULL,
	0 /* no priority */
};
#endif

/*
 * module init, varies a bit upon versions
 */
#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,5,56)))
/* at least, lxr doesn't have further kernels */
int __init fkt_init(void)
{
	static struct completion startup __initdata = COMPLETION_INITIALIZER(startup);
	if (register_blkdev(FKT_MAJOR,"fkt",&fkt_ops)) {
		printk(KERN_ERR "fkt: Unable to get major %d\n",FKT_MAJOR);
		return -EBUSY;
	}

	fkt_sendfile = sendfile;

#else
static struct gendisk *disk;
int __init fkt_init(void)
{
	static struct completion startup __initdata = COMPLETION_INITIALIZER(startup);
	/* 2.5.70 apparently needs this */
	
	if (!(disk = alloc_disk(1))) { /* only one fkt device */
		printk(KERN_ERR "fkt: No memory for block device\n");
		return -ENOMEM;
	}

	if (register_blkdev(FKT_MAJOR,"fkt")) {
		printk(KERN_ERR "fkt: Unable to get major %d\n",FKT_MAJOR);
		put_disk(disk);
		return -EBUSY;
	}

	disk->major = FKT_MAJOR;
	disk->first_minor = 0;
	disk->fops = &fkt_ops;
	sprintf(disk->disk_name, "fkt");
	add_disk(disk);
#endif

#ifdef DEBUG
	notifier_chain_register(&panic_notifier_list, &fkt_panic_block);
#endif
#if (LINUX_VERSION_CODE >= (KERNEL_VERSION(2,6,4)))
	kthread_create(fsync_thread,&startup,"fkt_flush");
#else
	kernel_thread(fsync_thread,&startup,0);
#endif
	wait_for_completion(&startup);

	printk("fkt: ok\n");
	return 0;
}

#ifdef MODULE
void cleanup_module(void)
{
	unsigned long buf;
	/* kill fsync thread */
	if (fsync_task); {
		set_tsk_thread_flag(fsync_task, TIF_SIGPENDING);
		wake_up_process(fsync_task);
	}
#if (LINUX_VERSION_CODE < (KERNEL_VERSION(2,5,56)))
	fkt_sendfile = NULL;
#endif
	unregister_blkdev(FKT_MAJOR,"fkt");
#if (LINUX_VERSION_CODE >= (KERNEL_VERSION(2,5,56)))
	del_gendisk(disk);
	put_disk(disk);
#endif
	down(&fkt_sem);
	fkt_active = 0;	/* just to be sure no probe will go */
	free_buffer(pagebuf,nbpages);
	buf = (unsigned long) rotatingbuf;
	rotatingbuf = NULL;
	free_page(buf);
	buf = (unsigned long) pagebuf,
	pagebuf = NULL;
	free_page(buf);
	up(&fkt_sem);
#ifdef DEBUG
	notifier_chain_unregister(&panic_notifier_list, &fkt_panic_block);
#endif
	down(&fsync_ended);
}
MODULE_AUTHOR("Samuel Thibault");
MODULE_SUPPORTED_DEVICE("fkt");
MODULE_LICENSE("GPL");
#endif

module_init(fkt_init);
