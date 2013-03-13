/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE 1 /* or _BSD_SOURCE or _SVID_SOURCE */
#define _REENTRANT

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>


#include "stdio_ev_codes.h"
#include "eztrace.h"
#include "pptrace.h"
/* set to 1 when all the hooks are set.
 * This is usefull in order to avoid recursive calls
 */
static int __stdio_initialized = 0;

ssize_t (*libread)(int fd, void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count)
{
  FUNCTION_ENTRY;
  INTERCEPT("read", libread);
  EZTRACE_EVENT3(FUT_STDIO_READ_START, fd, buf, count);
  ssize_t ret = libread(fd, buf, count);
  EZTRACE_EVENT3(FUT_STDIO_READ_STOP, fd, buf, count);
  return ret;
}

ssize_t (*libwrite)(int fd, const void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count)
{
  FUNCTION_ENTRY;
  INTERCEPT("write", libwrite);
  EZTRACE_EVENT3(FUT_STDIO_WRITE_START, fd, buf, count);
  ssize_t ret = libwrite(fd, buf, count);
  EZTRACE_EVENT3(FUT_STDIO_WRITE_STOP, fd, buf, count);

  return ret;
}

/* pointers to actual stdio functions */
ssize_t (*libpread)(int fd, void *buf, size_t count, off_t offset);
ssize_t (*libpwrite)(int fd, const void *buf, size_t count, off_t offset);
ssize_t (*libreadv)(int fd, const struct iovec *iov, int iovcnt);
ssize_t (*libwritev)(int fd, const struct iovec *iov, int iovcnt);
size_t  (*libfread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t  (*libfwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream);

off_t (*liblseek)(int fd, off_t offset, int whence);

int (*libselect)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int (*libpselect)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
		  const struct timespec *timeout, const sigset_t *sigmask);

int (*libpoll)(struct pollfd *fds, nfds_t nfds, int timeout);
int (*libppoll)(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask);


ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
  FUNCTION_ENTRY;
  INTERCEPT("pread", libpread);
  EZTRACE_EVENT4(FUT_STDIO_PREAD_START, fd, buf, count, offset);
  ssize_t ret = libpread(fd, buf, count, offset);
  EZTRACE_EVENT4(FUT_STDIO_PREAD_STOP, fd, buf, count, offset);
  return ret;
}

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
  FUNCTION_ENTRY;
  INTERCEPT("pwrite", libpwrite);
  EZTRACE_EVENT4(FUT_STDIO_PWRITE_START, fd, buf, count, offset);
  ssize_t ret = libpwrite(fd, buf, count, offset);
  EZTRACE_EVENT4(FUT_STDIO_PWRITE_STOP, fd, buf, count, offset);
  return ret;
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
{
  FUNCTION_ENTRY;
  INTERCEPT("readv", libreadv);
  EZTRACE_EVENT3(FUT_STDIO_READV_START, fd, iov, iovcnt);
  ssize_t ret = libreadv(fd, iov, iovcnt);
  EZTRACE_EVENT3(FUT_STDIO_READV_STOP, fd, iov, iovcnt);
  return ret;
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
  FUNCTION_ENTRY;
  INTERCEPT("writev", libwritev);
  EZTRACE_EVENT3(FUT_STDIO_WRITEV_START, fd, iov, iovcnt);
  ssize_t ret = libwritev(fd, iov, iovcnt);
  EZTRACE_EVENT3(FUT_STDIO_WRITEV_STOP, fd, iov, iovcnt);
  return ret;
}

size_t  fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  FUNCTION_ENTRY;
  INTERCEPT("fread", libfread);
  EZTRACE_EVENT4(FUT_STDIO_FREAD_START, ptr, size, nmemb, stream);
  size_t  ret =libfread(ptr, size, nmemb, stream);
  EZTRACE_EVENT4(FUT_STDIO_FREAD_STOP, ptr, size, nmemb, stream);
  return ret;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  FUNCTION_ENTRY;
  INTERCEPT("fwrite", libfwrite);
  EZTRACE_EVENT4(FUT_STDIO_FWRITE_START, ptr, size, nmemb, stream);
  size_t ret =libfwrite(ptr, size, nmemb, stream);
  EZTRACE_EVENT4(FUT_STDIO_FWRITE_STOP, ptr, size, nmemb, stream);
  return ret;
}


off_t lseek(int fd, off_t offset, int whence)
{
  FUNCTION_ENTRY;
  static int not_yet_init = 1;
  if(not_yet_init) {
    INTERCEPT("lseek", liblseek);
    not_yet_init = 0;
  }

  EZTRACE_EVENT3(FUT_STDIO_LSEEK_START, fd, offset, whence);
  off_t ret = liblseek(fd, offset, whence);
  EZTRACE_EVENT3(FUT_STDIO_LSEEK_STOP, fd, offset, whence);

  return ret;
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
  FUNCTION_ENTRY;
  INTERCEPT("select", libselect);
  EZTRACE_EVENT3(FUT_STDIO_SELECT_START, nfds, timeout?timeout->tv_sec : 0, timeout?timeout->tv_usec : 0);
  int ret = libselect(nfds, readfds, writefds, exceptfds, timeout);
  EZTRACE_EVENT3(FUT_STDIO_SELECT_STOP, nfds, timeout?timeout->tv_sec : 0, timeout?timeout->tv_usec : 0);
  return ret;
}

int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	    const struct timespec *timeout, const sigset_t *sigmask)
{
  FUNCTION_ENTRY;
  INTERCEPT("pselect", libpselect);
  EZTRACE_EVENT3(FUT_STDIO_PSELECT_START, nfds, timeout?timeout->tv_sec : 0, timeout?timeout->tv_nsec : 0);
  int ret = libpselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
  EZTRACE_EVENT3(FUT_STDIO_PSELECT_STOP, nfds, timeout?timeout->tv_sec : 0, timeout?timeout->tv_nsec : 0);
  return ret;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
  FUNCTION_ENTRY;
  INTERCEPT("poll", libpoll);
  EZTRACE_EVENT2(FUT_STDIO_POLL_START, nfds, timeout);
  int ret = libpoll(fds, nfds, timeout);
  EZTRACE_EVENT2(FUT_STDIO_POLL_STOP, nfds, timeout);

  return ret;
}

int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask)
{
  FUNCTION_ENTRY;
  INTERCEPT("ppoll", libppoll);
  EZTRACE_EVENT3(FUT_STDIO_PPOLL_START, nfds, timeout?timeout->tv_sec : 0, timeout?timeout->tv_nsec : 0);
  int ret = libppoll(fds, nfds, timeout, sigmask);
  EZTRACE_EVENT3(FUT_STDIO_PPOLL_STOP, nfds, timeout?timeout->tv_sec : 0, timeout?timeout->tv_nsec : 0);
  return ret;
}

START_INTERCEPT
  INTERCEPT2("read", libread)
  INTERCEPT2("write", libwrite)
  INTERCEPT2("pread", libpread)
  INTERCEPT2("pwrite", libpwrite)
  INTERCEPT2("readv", libreadv)
  INTERCEPT2("writev", libwritev)
  INTERCEPT2("fread", libfread)
  INTERCEPT2("fwrite", libfwrite)
  INTERCEPT2("lseek", liblseek)
  INTERCEPT2("select", libselect)
  INTERCEPT2("pselect", libpselect)

  INTERCEPT2("poll", libpoll)
  INTERCEPT2("ppoll", libppoll)
END_INTERCEPT

static void __stdio_init (void) __attribute__ ((constructor));
static void
__stdio_init (void)
{
  DYNAMIC_INTERCEPT_ALL();

#ifdef EZTRACE_AUTOSTART
  eztrace_start();
#endif
  __stdio_initialized = 1;
}

static void __stdio_conclude (void) __attribute__ ((destructor));
static void
__stdio_conclude (void)
{
  __stdio_initialized = 0;
  eztrace_stop();
}
