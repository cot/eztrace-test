/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <GTG.h>

#include "eztrace_convert.h"
#include "stdio_ev_codes.h"
#include "eztrace_list.h"

static int recording_stats = 0;

#define STDIO_CHANGE() if(!recording_stats) CHANGE()

#define TOSTRING(x) #x
#define GENERATE_HANDLER(fname)						\
  void handle_stdio_##fname (int start)					\
  {									\
    FUNC_NAME;								\
    DECLARE_THREAD_ID_STR(thread_id, CUR_INDEX, CUR_THREAD_ID);		\
    if(start) {								\
      STDIO_CHANGE() pushState(CURRENT, "ST_Thread", thread_id, TOSTRING(stdio_##fname)); \
    }else{								\
      STDIO_CHANGE() popState(CURRENT, "ST_Thread", thread_id);		\
    }									\
  }

GENERATE_HANDLER(read)
GENERATE_HANDLER(pread)
GENERATE_HANDLER(readv)
GENERATE_HANDLER(fread)

GENERATE_HANDLER(write)
GENERATE_HANDLER(pwrite)
GENERATE_HANDLER(writev)
GENERATE_HANDLER(fwrite)

GENERATE_HANDLER(select)
GENERATE_HANDLER(pselect)
GENERATE_HANDLER(lseek)
GENERATE_HANDLER(poll)
GENERATE_HANDLER(ppoll)

int
eztrace_convert_stdio_init()
{
  if(get_mode() == EZTRACE_CONVERT) {
    addEntityValue("stdio_read", "ST_Thread", "stdio_read", GTG_BLACK);
    addEntityValue("stdio_pread", "ST_Thread", "stdio_pread", GTG_BLACK);
    addEntityValue("stdio_readv", "ST_Thread", "stdio_readv", GTG_BLACK);
    addEntityValue("stdio_fread", "ST_Thread", "stdio_fread", GTG_BLACK);

    addEntityValue("stdio_write", "ST_Thread", "stdio_write", GTG_BLACK);
    addEntityValue("stdio_pwrite", "ST_Thread", "stdio_pwrite", GTG_BLACK);
    addEntityValue("stdio_writev", "ST_Thread", "stdio_writev", GTG_BLACK);
    addEntityValue("stdio_fwrite", "ST_Thread", "stdio_fwrite", GTG_BLACK);

    addEntityValue("stdio_select", "ST_Thread", "stdio_select", GTG_BLACK);
    addEntityValue("stdio_pselect", "ST_Thread", "stdio_pselect", GTG_BLACK);
    addEntityValue("stdio_lseek", "ST_Thread", "stdio_lseek", GTG_BLACK);
    addEntityValue("stdio_poll", "ST_Thread", "stdio_poll", GTG_BLACK);
    addEntityValue("stdio_ppoll", "ST_Thread", "stdio_ppoll", GTG_BLACK);
  }
  return 0;
}

/* return 1 if the event was handled */
int
handle_stdio_events(struct fxt_ev_64 *ev)
{
  if(!STARTED)
    return 0;

  switch(ev->code)
    {

    case FUT_STDIO_READ_START:    handle_stdio_read(1); break;
    case FUT_STDIO_PREAD_START:   handle_stdio_pread(1); break;
    case FUT_STDIO_READV_START:   handle_stdio_readv(1); break;
    case FUT_STDIO_FREAD_START:   handle_stdio_fread(1); break;
    case FUT_STDIO_WRITE_START:   handle_stdio_write(1); break;
    case FUT_STDIO_PWRITE_START:  handle_stdio_pwrite(1); break;
    case FUT_STDIO_WRITEV_START:  handle_stdio_writev(1); break;
    case FUT_STDIO_FWRITE_START:  handle_stdio_fwrite(1); break;
    case FUT_STDIO_SELECT_START:  handle_stdio_select(1); break;
    case FUT_STDIO_PSELECT_START: handle_stdio_pselect(1); break;
    case FUT_STDIO_LSEEK_START:   handle_stdio_lseek(1); break;
    case FUT_STDIO_POLL_START:    handle_stdio_poll(1); break;
    case FUT_STDIO_PPOLL_START:   handle_stdio_ppoll(1); break;


    case FUT_STDIO_READ_STOP:    handle_stdio_read(0); break;
    case FUT_STDIO_PREAD_STOP:   handle_stdio_pread(0); break;
    case FUT_STDIO_READV_STOP:   handle_stdio_readv(0); break;
    case FUT_STDIO_FREAD_STOP:   handle_stdio_fread(0); break;
    case FUT_STDIO_WRITE_STOP:   handle_stdio_write(0); break;
    case FUT_STDIO_PWRITE_STOP:  handle_stdio_pwrite(0); break;
    case FUT_STDIO_WRITEV_STOP:  handle_stdio_writev(0); break;
    case FUT_STDIO_FWRITE_STOP:  handle_stdio_fwrite(0); break;
    case FUT_STDIO_SELECT_STOP:  handle_stdio_select(0); break;
    case FUT_STDIO_PSELECT_STOP: handle_stdio_pselect(0); break;
    case FUT_STDIO_LSEEK_STOP:   handle_stdio_lseek(0); break;
    case FUT_STDIO_POLL_STOP:    handle_stdio_poll(0); break;
    case FUT_STDIO_PPOLL_STOP:   handle_stdio_ppoll(0); break;

    default:
      return 0;
    }
  return 1;
}

int
handle_stdio_stats(struct fxt_ev_64 *ev)
{
  recording_stats = 1;
  return handle_stdio_events(ev);
}

void print_stdio_stats() {
  printf ( "\nSTDIO:\n");
  printf   ( "-------\n");
}

struct eztrace_convert_module stdio_module;

void libinit(void) __attribute__ ((constructor));
void libinit(void)
{
  stdio_module.api_version = EZTRACE_API_VERSION;
  stdio_module.init = eztrace_convert_stdio_init;
  stdio_module.handle = handle_stdio_events;
  stdio_module.handle_stats = handle_stdio_stats;
  stdio_module.print_stats = print_stdio_stats;

  stdio_module.module_prefix = STDIO_EVENTS_ID;
  asprintf(&stdio_module.name, "stdio");
  asprintf(&stdio_module.description, "Module for stdio functions (read, write, select, poll, etc.)");

  stdio_module.token.data = &stdio_module;
  eztrace_convert_register_module(&stdio_module);
}

void libfinalize(void) __attribute__ ((destructor));
void libfinalize(void)
{
}
