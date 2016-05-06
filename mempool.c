//
// Created by mofan on 7/16/15.
//

#include "mempool.h"
#include "defs.h"


wxsvr_mempool_block_t* wxsvr_createblock(size_t blocksize) {
	wxsvr_mempool_block_t *b = NULL;
	if (0!=posix_memalign((void **)&b, WXSVR_MEMPOOL_POOL_ALIGNMENT, blocksize)) {
		wxsvr_err("0!=posix_memalign((void **)&b, WXSVR_MEMPOOL_POOL_ALIGNMENT, blocksize)");
		return NULL;
	}
	b->end = (char *)b + blocksize;
	b->last = WXSVR_MEMPOOL_ALIGN_PTR((char *) b + sizeof(wxsvr_mempool_block_t), WXSVR_MEMPOOL_ALIGNMENT);
	b->failed = 0;
	b->next = NULL;
	return b;
}

wxsvr_mempool_t* wxsvr_mempool_new(size_t blocksize) {
	wxsvr_mempool_t* mp = (wxsvr_mempool_t*)malloc(sizeof(wxsvr_mempool_t));
	mp->blockhead = wxsvr_createblock(blocksize);
	if (NULL==mp->blockhead) {
		wxsvr_err("NULL==mp->blockhead");
		return NULL;
	}
	mp->maxalloc = mp->blockhead->end - mp->blockhead->last;
	mp->block = mp->blockhead;
	mp->largehead = mp->large = NULL;
	mp->cleanup = NULL;
	return mp;
}

wxsvr_mempool_large_t* wxsvr_createlarge(size_t size) {
	wxsvr_mempool_large_t *l = (wxsvr_mempool_large_t*)malloc(sizeof(wxsvr_mempool_large_t) + size);
	if (NULL==l){
		wxsvr_err("NULL==l");
		return NULL;
	}
	l->ptr = (char *)l + sizeof(wxsvr_mempool_large_t);
	l->next = NULL;
	return l;
}

char* wxsvr_mempool_alloc(wxsvr_mempool_t* mp, size_t size) {
	if (size <= mp->maxalloc) {
		wxsvr_mempool_block_t *block4alloc = mp->block;
		wxsvr_mempool_block_t *lastnotnull;
		do {
			if (block4alloc->failed < 4) {
				if (size > (size_t) (block4alloc->end - block4alloc->last)) {
					block4alloc->failed++;
					lastnotnull = block4alloc;
					block4alloc = block4alloc->next;
				} else {
					//分配，对齐last，返回
					char *start = block4alloc->last;
					block4alloc->last = WXSVR_MEMPOOL_ALIGN_PTR(block4alloc->last + size, WXSVR_MEMPOOL_ALIGNMENT);
					return start;
				}
			} else {
				mp->block = block4alloc->next;
				lastnotnull = block4alloc;
				block4alloc = block4alloc->next;
			}
		} while (NULL != block4alloc);
		//创建新块
		block4alloc = wxsvr_createblock(mp->blockhead->end - (char *)mp->blockhead);
		lastnotnull->next = block4alloc;
		if (mp->block == NULL) {
			mp->block = block4alloc;
		}
		//分配，对齐last，返回
		char *start = block4alloc->last;
		block4alloc->last = WXSVR_MEMPOOL_ALIGN_PTR(block4alloc->last + size, WXSVR_MEMPOOL_ALIGNMENT);
		return start;
	} else {
		//直接分配大块内存,返回
		wxsvr_mempool_large_t *l = wxsvr_createlarge(size);
		if (l==NULL) {
			wxsvr_err("NULL==l");
			return NULL;
		}
		if (mp->largehead == NULL) {
			mp->largehead = l;
		} else {
			mp->large->next = l;
		}
		mp->large = l;
		return l->ptr;
	}
}

void wxsvr_mempool_free(wxsvr_mempool_t* mp){
	if (mp==NULL){
		return;
	}
	while (mp->cleanup != NULL) {
		mp->cleanup->cleanup_cb(mp->cleanup->arg);
		mp->cleanup = mp->cleanup->next;
	}
	wxsvr_mempool_large_t *tmpLarge;
	while (mp->largehead != NULL) {
		tmpLarge = mp->largehead->next;
		free(mp->largehead);
		mp->largehead = tmpLarge;
	}
	wxsvr_mempool_block_t *tmpBlock;
	while (mp->blockhead != NULL) {
		tmpBlock = mp->blockhead->next;
		free(mp->blockhead);
		mp->blockhead = tmpBlock;
	}
	mp->block = NULL;
	mp->large = NULL;
	free(mp);
	mp=NULL;
}