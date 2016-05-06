#ifndef WXSVR_DEFS_H
#define WXSVR_DEFS_H


#include <stdio.h>
#include <stdint.h>


#define wxsvr_err(pstr) fprintf(stderr,"[%s:%d]%s\n",__FILE__,__LINE__,(pstr))
#define container_of(ptr, type, field) ((type*)((char*)(ptr) - ((char*)&((type*)0)->field)))


#endif