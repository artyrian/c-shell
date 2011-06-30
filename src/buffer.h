#ifndef _BUFFER_H_
#define _BUFFER_H_


typedef struct 
{
	char * string;
	int cnt;
	int part;
} Buffer;


void init_buffer (Buffer *);
void add_symbol (int, Buffer *);
void free_buffer (Buffer *);


#endif
