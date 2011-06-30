#include "buffer.h"

#include <stdlib.h>
#include <string.h>


#define BUF_SIZE 32 


void extend_buffer (Buffer *);


/* Initialization buffer.
 * Create string for write
 * and init cnts of parts(1) and symbols(0)
 */
void init_buffer (Buffer * buf)
{
	buf->string = (char *) malloc (BUF_SIZE);
	buf->string[0] = '\0';
	buf->part = 1;
	buf->cnt = 0;
}




/* Extend buffer (linear).
 * Inc part of buf, 
 * create new string greater than was, 
 * copy old to new
 * delete old
 */
void extend_buffer (Buffer *buf)
{
	char * tmp_string;

	tmp_string = malloc ((++buf->part) * BUF_SIZE);
	strcpy (tmp_string, buf->string);

	free (buf->string);

	buf->string = tmp_string;
}


/*
 * Function, which add 1 symbol to buffer,
 * and extend it if necessary.
 */
void add_symbol (int c, Buffer * buf)
{
	if ( buf->cnt == BUF_SIZE * buf->part - 1 ) {
		extend_buffer (buf);
	}
	else {
		buf->string [buf->cnt++] = c;
		buf->string [buf->cnt] = '\0';
	}
}


/*
 *Free buffer
 */
 void free_buffer (Buffer * buf)
 {
	free (buf->string);
 }
