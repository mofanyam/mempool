//
// Created by mofan on 7/16/15.
//

#ifndef WXSVR_MEMPOOL_H
#define WXSVR_MEMPOOL_H

#include <stdlib.h>
//#include <jemalloc/jemalloc.h> /*must after #include <stdlib.h>*/
#include <stdint.h>


#define WXSVR_MEMPOOL_ALIGN_PTR(ptr, size)  (char *)(((uintptr_t)(ptr)+((uintptr_t)size-1))&~((uintptr_t)size-1))
#define WXSVR_MEMPOOL_ALIGNMENT sizeof(unsigned long)
#define WXSVR_MEMPOOL_POOL_ALIGNMENT 16

typedef struct wxsvr_mempool_block_s {
	char *last;
	char *end;
	struct wxsvr_mempool_block_s *next;
	size_t failed;
} wxsvr_mempool_block_t;

typedef struct wxsvr_mempool_large_s {
	char *ptr;
	struct wxsvr_mempool_large_s *next;
} wxsvr_mempool_large_t;

typedef void (*wxsvr_mempool_cleanup_cb)(void *arg);

typedef struct wxsvr_mempool_cleanup_s {
	wxsvr_mempool_cleanup_cb cleanup_cb;
	void *arg;
	struct wxsvr_mempool_cleanup_s *next;
} wxsvr_mempool_cleanup_t;

typedef struct wxsvr_mempool_s{
	wxsvr_mempool_block_t *blockhead,*block;
	wxsvr_mempool_large_t *largehead,*large;
	wxsvr_mempool_cleanup_t *cleanup;
	size_t maxalloc;
}wxsvr_mempool_t;


extern wxsvr_mempool_t* wxsvr_mempool_new(size_t blocksize);
extern void wxsvr_mempool_free(wxsvr_mempool_t* mp);
extern char* wxsvr_mempool_alloc(wxsvr_mempool_t* mp, size_t size);


#endif //WXSVR_MEMPOOL_H
