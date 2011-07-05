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
	QUOTE,
	BSLASH,
	BSLASH2
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
	LEX_QUOTE,
	LEX_BSLAH
} Type_lex;



/*
Structure of one element of list
*/
typedef struct TagListWord
{
	char * lexem;
	Type_lex t_lex;
	int lenght;
	struct TagListWord * next;
} ListWord;


/*
Structure of list elements.
*/
typedef struct TagList
{
	ListWord * cmd;
//	Type_lex t_lex;
	int count;
//	struct TagList * next;
} List;


void print_list (List *);
int init_list (List *);
int free_list (List *);
int get_symbol ();
int add_to_list (List *, ListWord *);// --
int fill_list (List *);	// +-
/*
<<<<<<< HEAD
ListWord * feed_symbol (int, State *, Buffer *);
ListWord * step (int c, State *, Buffer *);
ListWord * state_H (int, State *, Buffer *);
ListWord * state_WORD (int, State *, Buffer *);
ListWord * state_DELIM (int, State *, Buffer *);
=======
*/
ListElem * feed_symbol (int, State *, Buffer *);
ListElem * step (int c, State *, Buffer *);
ListElem * state_H (int, State *, Buffer *);
ListElem * state_WORD (int, State *, Buffer *);
ListElem * state_DELIM (int, State *, Buffer *);
ListElem * state_QUOTE (int, State *, Buffer *);
ListElem * state_BSLASH (int, State *, Buffer *);
ListElem * state_BSLASH2 (int, State *, Buffer *);
//>>>>>>> a55a597bda2247dbb2f11a200d7357f71f24d638

#endif
