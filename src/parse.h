#ifndef _PARSE_H_
#define _PARSE_H_

#include "buffer.h"

/*
Allowed states of auotomate.
*/
typedef enum
{
	H,
	WORD,
	DELIM,
	QUOTE
} State;


/*
Types of lexemes.
*/
typedef enum 
{
	LEX_NULL,
	LEX_WORD,
	LEX_COMMA,
	LEX_PIPE,
	LEX_READ,
	LEX_WRITE,
	LEX_APPEND,
	LEX_AND,
	LEX_OR,
	LEX_QUOTE
} Type_lex;



/*
Structure of one element of list
*/
typedef struct TagListElem
{
	char * lexem;
	Type_lex t_lex;
	int lenght;
	struct TagListElem * next;
} ListElem;


/*
Structure of list elements.
*/
typedef struct TagList
{
	ListElem * ptr;
	Type_lex t_lex;
	int count;
	struct TagList * next;
} List;


void print_list (List *);
int init_list (List *);
int free_list (List *);
int get_symbol ();
int add_to_list (List *, ListElem *);// --
int fill_list (List *);	// +-
ListElem * feed_symbol (int, State *, Buffer *);
ListElem * step (int c, State *, Buffer *);
ListElem * state_H (int, State *, Buffer *);
ListElem * state_WORD (int, State *, Buffer *);
ListElem * state_DELIM (int, State *, Buffer *);

#endif
