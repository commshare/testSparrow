#ifndef _SC_BUFFER_H_
#define _SC_BUFFER_H_

#include"sc_lock.h"

typedef struct{
	uint8_t *data;
	int size;
	int level;
	uint8_t *rd_ptr;
	uint8_t *wr_ptr;
	sc_lock_t mutex;
}sc_buf_t;
int buf_init(sc_buf_t *buf,int size);

#endif
