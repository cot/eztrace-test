/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE 1 /* or _BSD_SOURCE or _SVID_SOURCE */
#define _REENTRANT

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

#include "memory_ev_codes.h"
#include "eztrace.h"
#include "pptrace.h"

/* set to 1 when all the hooks are set.
 * This is usefull in order to avoid recursive calls
 */
static int __memory_initialized = 0;

#define MAGIC_PATTERN 0xdeadbeef

/* when a buffer is allocated with eztrace, it has the following pattern:
 * PADDING BLOCK_INFO USER_BLOCK
 * block_info has to be right-aligned because of calloc.
 * the address should be a multiple of 8 bytes in order to avoid bugs
 * (if not aligned, some weird bugs may happen when using -O3)
 */
enum __memory_type {
  MEM_TYPE_MALLOC,
  MEM_TYPE_CUSTOM_MALLOC
};

/* todo: we could add information like:
 * - date of malloc
 * - thread id that allocated the block
 * - NUMA node ?
 */
struct mem_block_info {
  void* u_ptr;		/* address of the user block */
  void* p_ptr;		/* address of the padding */

  enum __memory_type mem_type;

  size_t total_size; 		/* size allocated (including this structure) */
  size_t size;			/* size of the buffer (not including this structure) */

  /* WARNING: this must be the last field of the structure */
  uint32_t canary;		/* this is used for checking that we malloc'ed the buffer */
}__attribute__((__packed__));

/* size of the padding + mem_info structure */
#define GET_HEADER_SIZE()     (sizeof(struct mem_block_info))

#define CANARY_OK(u_ptr) ((*(uint32_t*)((u_ptr) - sizeof(uint32_t))) == MAGIC_PATTERN)

/* converts a pointer to a user_block into a pointer to the block info */
#define USER_PTR_TO_BLOCK_INFO(u_ptr, b_ptr)				\
  do {									\
    if(! CANARY_OK(u_ptr)) {						\
      /* we didn't malloc this buffer */				\
      (b_ptr) = NULL;							\
      break;								\
    }									\
    b_ptr = (void*)(ptr - (void*)sizeof(struct mem_block_info));	\
  }while(0)

/* converts a pointer to a user_block into a pointer to the padding */
#define USER_PTR_TO_PADDING(u_ptr, p_ptr)	\
  do {						\
    struct mem_block_info *b_ptr;		\
    USER_PTR_TO_BLOCK_INFO(u_ptr, b_ptr);	\
    if(!b_ptr) {				\
      (p_ptr) = NULL;				\
      break;					\
    }						\
    (p_ptr) = b_ptr->p_ptr;			\
  } while(0)


/* fill a mem_info structure
 * @param p_mem the mem_info* structure to fill
 * @param ptr the address returned by (m,c,re)alloc
 * @param nmemb the number of elements
 * @param block_size the size of 1 element
 */
#define INIT_MEM_INFO(p_mem, ptr, nmemb, block_size)		\
  do {								\
    unsigned int nb_memb_header = GET_HEADER_SIZE() / block_size;	\
    if(block_size*nb_memb_header < GET_HEADER_SIZE())		\
      nb_memb_header++;						\
    void* u_ptr = ptr + (block_size*nb_memb_header);		\
    p_mem = u_ptr - sizeof(struct mem_block_info);		\
    p_mem->p_ptr = ptr;						\
    p_mem->total_size = (nmemb + nb_memb_header) * block_size;	\
    p_mem->size = nmemb * block_size;				\
    p_mem->u_ptr = u_ptr;					\
    p_mem->canary = MAGIC_PATTERN;				\
  } while(0)



/* todo: also implement mmap and munmap ?
 */
void* (*libcalloc)(size_t nmemb, size_t size);
void* (*libmalloc)(size_t size) = NULL;
void  (*libfree)(void *ptr);
void* (*librealloc)(void *ptr, size_t size);

static int malloc_protect_on = 0;

/* Custom malloc function. It is used when libmalloc=NULL (ie. during startup)
 * This function is not thread-safe and is very likely to be bogus, so use with caution
 */
static void* hand_made_malloc(size_t size)
{
  /* allocate a 1MB buffer */
#define POOL_SIZE (1024*1024)
  static char mem[POOL_SIZE] = { '\0' };

  /* since this function is only used before we found libmalloc, there's no fancy
   * memory management mechanism (block reuse, etc.)
   */
  static char* next_slot = &mem[0];
  static int total_alloc = 0;

  if(libmalloc)
    /* let's use the real malloc */
    return malloc(size);

  struct mem_block_info *p_block = NULL;
  INIT_MEM_INFO(p_block, next_slot, size, 1);

  p_block->mem_type = MEM_TYPE_CUSTOM_MALLOC;
  total_alloc += size;
  next_slot = next_slot + p_block->total_size;

  return p_block->u_ptr;
}

void* malloc(size_t size)
{
  /* if memory_init hasn't been called yet, we need to get libc's malloc address */

  if (!libmalloc) {
    if(malloc_protect_on)
      /* protection flag says that malloc is already trying to retrieve the address of malloc.
       * if we call dlsym now, there will be an infinite recursion, so let's allocate
       * memory 'by hand'
       */
      return hand_made_malloc(size);

    /* set the protection flag and retrieve the address of malloc.
     * if dlsym calls malloc, memory will be allocated 'by hand'
     */
    malloc_protect_on = 1;
    libmalloc = dlsym(RTLD_NEXT, "malloc");
    char* error;
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
    /* it is now safe to call libmalloc */
    malloc_protect_on = 0;
  }

  EZTRACE_PROTECT {
    void* pptr = libmalloc(size + GET_HEADER_SIZE());
    struct mem_block_info *p_block = NULL;
    INIT_MEM_INFO(p_block, pptr, size, 1);
    p_block->mem_type = MEM_TYPE_MALLOC;

    EZTRACE_EVENT2(FUT_MEMORY_MALLOC, p_block->size, p_block->u_ptr);

#if 0
    /* for debugging purpose only */
    uint32_t* canary = p_block->u_ptr-sizeof(uint32_t);
    if(*canary != MAGIC_PATTERN) {
      fprintf(stderr, "warning: canary = %x instead of %x\n", *canary, MAGIC_PATTERN);
    }
#endif

    return p_block->u_ptr;
  }
  return libmalloc(size);
}

void* realloc(void *ptr, size_t size)
{

  /* if ptr is NULL, realloc behaves like malloc */
  if(!ptr)
    return malloc(size);

  /* if size=0 and ptr isn't NULL, realloc behaves like free */
  if(!size && ptr) {
    free(ptr);
    return NULL;
  }

  if (!librealloc) {
    librealloc = dlsym(RTLD_NEXT, "realloc");
    char* error;
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
  }

  if(!CANARY_OK(ptr)) {
    /* we didn't malloc'ed this buffer */
    return librealloc(ptr, size);
  }

  EZTRACE_PROTECT {
    struct mem_block_info *p_block;
    USER_PTR_TO_BLOCK_INFO(ptr, p_block);
    size_t old_size = p_block->size;
    size_t header_size = p_block->total_size - p_block->size;

    if(p_block->mem_type != MEM_TYPE_MALLOC) {
      fprintf(stderr, "Warning: realloc a ptr that was allocated by hand_made_malloc\n");
    }

    void *pptr = librealloc(p_block->p_ptr, size + header_size);

    if(!p_block) {
      return NULL;
    }

    INIT_MEM_INFO(p_block, pptr, size+header_size, 1);

    p_block->mem_type = MEM_TYPE_MALLOC;

    EZTRACE_EVENT3(FUT_MEMORY_REALLOC, old_size, p_block->size, p_block->u_ptr);
    return p_block->u_ptr;
  }

  return librealloc(ptr, size);
}

void* calloc(size_t nmemb, size_t size)
{
  if (!libcalloc) {
    void* ret = malloc(nmemb*size);
    if(ret) {
      memset(ret, 0, nmemb*size);
    }
    return ret;
  }

  EZTRACE_PROTECT {
    /* compute the number of blocks for header */
    int nb_memb_header = sizeof(struct mem_block_info) / size;
    if(size*nb_memb_header < sizeof(struct mem_block_info))
      nb_memb_header++;

    /* allocate buffer + header */
    void* p_ptr = libcalloc(nmemb + nb_memb_header, size);

    struct mem_block_info *p_block = NULL;
    INIT_MEM_INFO(p_block, p_ptr, nmemb, size);
    p_block->mem_type = MEM_TYPE_MALLOC;

    EZTRACE_EVENT2(FUT_MEMORY_MALLOC, p_block->size, p_block->u_ptr);

#if 0
    /* for debugging purpose only */
    uint32_t* canary = p_block->u_ptr-sizeof(uint32_t);
    if(*canary != MAGIC_PATTERN) {
      fprintf(stderr, "warning: canary = %x instead of %x\n", *canary, MAGIC_PATTERN);
    }
#endif

    return p_block->u_ptr;
  }
  return libcalloc(nmemb, size);
}

void free(void* ptr)
{
  if (!libfree) {
    libfree = dlsym(RTLD_NEXT, "free");
    char* error;
    if ((error = dlerror()) != NULL) {
      fputs(error, stderr);
      exit(1);
    }
  }

  if(!ptr) {
      libfree(ptr);
      return ;
  }

  /* first, check wether we malloc'ed the buffer */
  if(!CANARY_OK(ptr)){
    /* we didn't malloc this buffer */
    libfree(ptr);
    return ;
  }

  /* retrieve the block information and free it */
  EZTRACE_PROTECT {
    struct mem_block_info *p_block;
    USER_PTR_TO_BLOCK_INFO(ptr, p_block);

    if(p_block->mem_type == MEM_TYPE_MALLOC) {
      EZTRACE_EVENT2(FUT_MEMORY_FREE, p_block->size, p_block->u_ptr);
      libfree(p_block->p_ptr);
    } else {
      /* the buffer was allocated by hand_made_malloc, there's nothing to free */
    }
  }
}

START_INTERCEPT
  INTERCEPT2("malloc", libmalloc)
  INTERCEPT2("calloc", libcalloc)
  INTERCEPT2("realloc", librealloc)
  INTERCEPT2("free", libfree)
END_INTERCEPT

static void __memory_init (void) __attribute__ ((constructor));
static void
__memory_init (void)
{

 DYNAMIC_INTERCEPT_ALL();
#ifdef EZTRACE_AUTOSTART
  eztrace_start ();
#endif
  __memory_initialized = 1;
}

static void __memory_conclude (void) __attribute__ ((destructor));
static void
__memory_conclude (void)
{
  __memory_initialized = 0;
  eztrace_stop ();
}
