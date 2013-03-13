/*	fkt_record.c

	fkt_record --	control program to allow a user to trace his/her programs
					with fkt (fast kernel tracing) in the kernel.

	Copyright (C) 2000, 2001  Robert D. Russell -- rdr@unh.edu
	2003, 2004, 2012  Samuel Thibault -- samuel.thibault@ens-lyon.org

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


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/sendfile.h>
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#include <sys/vfs.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/fkt.h>
#include "timer.h"
#include "fxt.h"

#include "fxt-tools.h"

#ifndef WHITE_SPACE
#define WHITE_SPACE	" \t\n\f\v\r"
#endif

#define OPTIONS ":f:k:s:p:dhnS:"
#define FKT_DEV "/dev/fkt"
#define MAXCPUS	16


static int fd,fkt;
static int kpid = -1;
static size_t size = 0;

struct code_list_item	{
	unsigned int			code;
	char					*name;
	struct code_list_item	*link;
};

void dumptime( char *message , time_t *the_time, clock_t *the_jiffies )
	{
	struct tms		cur_time;
	struct tm		*breakout;
	char			buffer[128];

	if( (*the_time = time(NULL)) == -1 )
		perror("time");

	if( (*the_jiffies = times(&cur_time)) == (clock_t) -1 )
		perror("times");

	if( (breakout = localtime(the_time)) == NULL )
		fprintf(stderr, "Unable to break out current time\n");

	if( strftime(buffer, 128, "%d-%b-%Y %H:%M:%S", breakout) == 0 )
		fprintf(stderr, "Unable to convert current time\n");

	printf("%7s: %s: user_time = %ld, system_time = %ld, jiffies = %ld\n",
			message, buffer,
			(long)cur_time.tms_utime, (long)cur_time.tms_stime,
			(long)*the_jiffies);
	fflush(stdout);
	}


double pr_times( FILE * fp, clock_t real, struct tms * tmsstart,
											struct tms * tmsend, int verbose )
	{
    static long clktck;
	double	elapsed;

    if (clktck == 0)
        if((clktck = sysconf(_SC_CLK_TCK)) < 0)
            perror("sysconf error");

	elapsed = ((double)real)/((double)clktck);
	if( verbose )
		{
		fprintf(fp, "Elapsed: %7.2f, ", elapsed);
		fprintf(fp, "user: %7.2f, ", (tmsend->tms_utime - tmsstart->tms_utime) /
			   (double) clktck);
		fprintf(fp, "sys:  %7.2f\n", (tmsend->tms_stime - tmsstart->tms_stime) /
			   (double) clktck);
		}
	return elapsed;
	}

/*	this is the stuff that will be traced if user doesn't supply anything else*/
pid_t do_some_stuff( void )
	{
	pid_t		kidpid;
	int			i, j, sum;
	struct tms	tmsstart, tmsstop;
	clock_t		start, stop;

	fflush(stdout);
	if( (kidpid = fork()) < 0 )
		{
		perror("fork");
		sleep(4);
		}
	else if( kidpid == 0 )
		{/* this is the kid */
		printf("job with pid %d started\n", (int)getpid());
		fflush(stdout);
		ioctl(fkt,FKT_WAITREADY);	/* wait for sendfile to settle down */
		if( (start = times(&tmsstart)) < 0 )
			exit(EXIT_FAILURE);
		for( i = 0;  i < 4;  i++ )
			{
			sum = 0;
			for( j = 0;  j < 1000000;  j++ )
				sum += j;
			printf("new kid: i = %d, j = %d\n", i, j);
			fflush(stdout);
			sleep(1);
			}
		if( (stop = times(&tmsstop)) < 0 )
			exit(EXIT_FAILURE);
		printf("job with pid %d finished\n", (int)getpid());
		fflush(stdout);
		pr_times(stdout, stop-start, &tmsstart, &tmsstop, 1);
		exit(EXIT_SUCCESS);
		}
	/*	this is the parent, just return the kid's pid */
	kpid = kidpid;
	return kidpid;
	}


/* this creates a job requested by user and traces that */
pid_t spawn_stuff( int argc, char *argv[] )
	{
	pid_t	kidpid;

	fflush(stdout);
	if( (kidpid = fork()) < 0 )
		{
		perror("fork");
		}
	else if( kidpid == 0 )
		{/* this is the kid */
			struct tms tms;
		ioctl(fkt,FKT_WAITREADY);	/* wait for sendfile to settle down */
		if (times(&tms)<0) {
			perror("times");
			exit(EXIT_FAILURE);
		}
		printf("executing %s\n",argv[optind]);
		execvp(argv[optind], &argv[optind]);
		perror(argv[optind]);
		exit(EXIT_FAILURE);
		}
	/*	this is the parent, just return the kid's pid */
	kpid = kidpid;
	return kidpid;
	}

/* handler for SIGINT */
void int_handler(int signo)
	{
	
	  printf("\n PARENT HANDLER");
	  fflush(stdout);
	  if(kpid != -1 && kill(kpid, SIGINT) == -1)
		{
		   perror("\nError in kill");
		}
	}

/* handler for child death, just to have sendfile interrupted */
void chld_handler(int signo)
{
	printf("\n Child died");
	fflush(stdout);
}

void usage(char *progname) {
	printf("\
Usage: %s [-k mask] [-f output-file] [-s size] [-S System.map] [-p pow]\n\
           [--] [program p1 p2 p3 ...]\n\
\n\
	if -f is not given, look for TRACE_FILE environment variable, and\n\
	if that is not given, write to \"trace_file\".\n\
\n\
	if -k is given, use the indicated mask to enable tracing.\n\
	if that is not given, use mask of 1 (only syscalls, irqs and traps).\n\
\n\
	if -s is not given or set to 0, the filesystem's size is used.\n\
	(K, M and G suffixes are accepted)\n\
\n\
	if -S is given, the given System.map is merged with /proc/kallsyms or\n\
	/proc/ksyms and recorded before the actual recording. An error is raised\n\
	if an inconsistency is detected (i.e. this System.map does not\n\
	correspond to the current kernel.\n\
\n\
	if -n is given, no program is launched at all, you have to press ^C to\n\
	interrupt recording\n\
\n\
	if -p is given, fkt will allocate 2^pow pages for its recording buffer\n\
	it is useful if load pikes are expected during the recording.\n\
	the default is 7.\n\
\n\
	if program is not given, just spawn a nonsense kid to burn cpu.\n\
",progname);
}

int main( int argc, char *argv[] )
	{
	pid_t				kidpid = 0, donepid = 0;
	int					status = 0;
	int					c, i;
	char				*outfile, *ptr;
	unsigned int		keymask;
	int					powpages=0,dma=0,nop=0;
	off_t				dummyoff=0;
	struct sigaction  	sig_act1;
	struct statfs		statfs;
	struct stat			st;
	ssize_t				ret;
	fxt_t				fxt;
	struct fxt_infos	*info;

	sig_act1.sa_handler = int_handler;
	sigemptyset(&sig_act1.sa_mask);
	sig_act1.sa_flags = 0;
	sigaddset(&sig_act1.sa_mask, SIGINT);

	outfile = NULL;
	keymask = 1;
	opterr = 0;

	if (!(fxt = fxt_setinfos(FXT_SPACE_KERNEL)))
		{
		perror("allocating fxt record");
		exit(EXIT_FAILURE);
		}

	while( (c = getopt(argc, argv, OPTIONS)) != EOF )
		switch( c )
			{
		case 'k':
			keymask = strtoll(optarg, &ptr, 0);
			if( strspn(ptr, WHITE_SPACE) != strlen(ptr) )
				{
				fprintf(stderr, "illegal keymask parameter %s\n", optarg);
				exit(EXIT_FAILURE);
				}
			break;
		case 'f':
			outfile = optarg;
			break;
		case 's': {
			size_t mult = 1;
			switch (optarg[strlen(optarg)-1]) {
			case 'K': case 'k': mult = 1 << 10; break;
			case 'M': case 'm': mult = 1 << 20; break;
			case 'G': case 'g': mult = 1 << 30; break;
			}
			if (mult>1) optarg[strlen(optarg)-1]='\0';
			size = strtol(optarg, &ptr, 0) * mult;
			if( strspn(ptr, WHITE_SPACE) != strlen(ptr) )
				{
				fprintf(stderr, "illegal size parameter %s\n", optarg);
				exit(EXIT_FAILURE);
				}
			break;
		}
		case 'p':
			powpages = strtol(optarg, &ptr, 0);
			if( strspn(ptr, WHITE_SPACE) != strlen(ptr) )
				{
				fprintf(stderr, "illegal power parameter %s\n", optarg);
				exit(EXIT_FAILURE);
				}
			break;
		case 'd':
			dma = 1;
			break;
		case 'n':
			nop = 1;
			break;
		case 'S':
			fxt_get_symbols(fxt,optarg,FXT_SYSMAP_FILE,0);
			break;
		case ':':
			fprintf(stderr, "missing parameter to switch %c\n", optopt);
			usage(argv[0]);
			exit(EXIT_FAILURE);
		case 'h':
			usage(argv[0]);
			exit(0);
		case '?':
			fprintf(stderr, "illegal switch %c\n", optopt);
			usage(argv[0]);
			exit(EXIT_FAILURE);
			}	/* switch */

	/*	get the cpu info here */
	info = fxt_infos(fxt);
	printf("%7s: %d\n", "n_cpus", info->ncpus);
	for( i = 0;  i < info->ncpus;  i++ )
		printf("  cpu %d: %d MHZ\n", i, info->mhz[i]);


	printf("%7s: %d\n", "pid", info->record_pid);

	if( outfile == NULL )
		{
		if( (outfile = getenv("TRACE_FILE")) == NULL )
			outfile = DEFAULT_TRACE_FILE;
		}
	while( (fd = open(outfile, O_RDWR|O_CREAT|O_TRUNC, 0666)) < 0 )
		{/* could not open the indicated file name for writing */
		perror(outfile);
		if( strcmp(outfile, DEFAULT_TRACE_FILE) == 0 )
			/* can't open the default file, have to give up */
			exit(EXIT_FAILURE);
		outfile = DEFAULT_TRACE_FILE;
		fprintf(stderr, "output defaulting to %s\n", outfile);
		}

	if( size == 0 )
		{
		if( fstat(fd, &st) )
			{
			perror("getting attributes of trace file");
			exit(EXIT_FAILURE);
			}
#ifdef BLKGETSIZE
		if( S_ISBLK(st.st_mode) )
			{
			int sectors;
			printf("block device\n");
			if( ioctl(fd,BLKGETSIZE,&sectors) )
				{
				perror("getting device size\n");
				exit(EXIT_FAILURE);
				}
			if( sectors >= 0xffffffffUL >> 9 )
				size=0xffffffffUL;
			else
				size = sectors << 9;
			}
		else 
#endif
			{
			if( fstatfs(fd, &statfs) )
				{
				perror("getting file system size\n");
				exit(EXIT_FAILURE);
				}
			if( statfs.f_bavail >= 0xffffffffUL / statfs.f_bsize )
				size = 0xffffffffUL;
			else
				size = statfs.f_bsize * statfs.f_bavail;
			}
		printf("%zuM\n",size>>20);
		}

	/*	force all I/O to be synchronized at this point */
	for( i = 0;  i <= fd;  i++ )
		fsync(i);

	if( (fkt = open(FKT_DEV, O_RDONLY|O_NONBLOCK)) < 0 ) {
		perror("open(\"" FKT_DEV "\")");
		exit(EXIT_FAILURE);
	}

	if( ioctl(fkt, FKT_SETINITMASK, keymask) < 0 ) {
		perror("setting initial mask");
		exit(EXIT_FAILURE);
	}

	if( powpages )
	if( ioctl(fkt, FKT_SETINITPOWPAGES, powpages) < 0 ) {
		perror("setting initial number of pages");
		exit(EXIT_FAILURE);
	}

	if( dma )
	if( ioctl(fkt, FKT_SETTRYDMA, dma) < 0 )
		perror("asking for trying to get dma");

	if(sigaction(SIGINT, &sig_act1, NULL))
		{
		  perror("\nsigaction SIGINT\n");
		}

	signal(SIGCHLD,chld_handler);

	if( !nop ) {
		if( optind < argc )
			kidpid = spawn_stuff(argc, argv);
		else
			kidpid = do_some_stuff();
	}

	/* record information as soon as now: in case we crash,
	 * only the stop_time & jiffies will be wrong (doesn't hurt that much
	 * for the analysis) */
	dumptime("start", &info->start_time, &info->start_jiffies);

	/* merge of System.map and /proc/ksyms */
	if( !stat("/proc/kallsyms",&st) )
		{/* on >=2.5 kernels, kallsyms gives everything we may want */
		fxt_get_symbols(fxt,"/proc/kallsyms",FXT_MODLIST,0);
		fxt_get_symbols(fxt,"/proc/kallsyms",FXT_PROC,0);
		}
	else
		{/* but on 2.4 kernels, only ksyms is available */
		fxt_get_symbols(fxt,"/proc/ksyms",FXT_MODLIST,0);
		fxt_get_symbols(fxt,"/proc/ksyms",FXT_PROC,0);
		}

	if (fxt_fdwrite(fxt,fd)<0)
		{
		perror("fxt_fdwrite");
		exit(EXIT_FAILURE);
		}
	if (fxt_fdevents_start(fxt,fd,FXT_TRACE_KERNEL_RAW)<0)
		{
		perror("fxt_fdstartevents");
		exit(EXIT_FAILURE);
		}

	/* ok, now we've consumed our quota, wait for a second to get back some
	 * priority for the recording. */

	sleep(1);
	printf("launching recording\n");
	ret = sendfile(fd, fkt, &dummyoff, size);
	if( ret < 0 )
		{
		perror("sendfile");
		fprintf(stderr,"hoping the trace wasn't lost\n");
		}

	dumptime("stop", &info->stop_time, &info->stop_jiffies);

	if( kidpid > 0 )
	{/* control came back with kidpid>0, child started ok, wait for it */
		if (!(donepid = waitpid(kidpid, &status, WNOHANG))) {

	/* kill kid if not dead already, since recording ended */
		  if(kill(kidpid, SIGINT) == -1)
			{
			   perror("\nError in kill");
			}

		  donepid = waitpid(kidpid, &status, 0);
		}
		/* print out stuff about the kid */
		kpid = kidpid;
		printf("job with pid %d started\n", (int)kidpid);
		if( donepid < 0 )
			perror("waitpid");
		else if( WIFEXITED(status) )
			printf("job with pid %d exited with status %d\n",
					(int)kidpid, WEXITSTATUS(status));
		else if( WIFSIGNALED(status) )
			printf("job with pid %d killed by signal %d\n",
					(int)kidpid, WTERMSIG(status));
		else
			printf("job with pid %d terminated for unknown reason\n",
					(int)kidpid);
		fflush(stdout);
	}

	if (fxt_fdevents_stop(fxt,fd)<0)
		{
		perror("fxt_fdstartevents");
		exit(EXIT_FAILURE);
		}

	if ( lseek(fd, 0, SEEK_SET) < 0 ) {
		perror("seeking back to the beginning");
		exit(EXIT_FAILURE);
	}
	if (fxt_fdwritetime(fxt, fd)<0)
		{
		perror("fxt_fdwritetime");
		exit(EXIT_FAILURE);
		}
	return 0;
	}

/* vim: ts=4
 */
