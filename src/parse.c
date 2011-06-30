#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


ListElem * new_element (Buffer * buf);


void print_list (List * list)
{
	ListElem * cur = list->ptr;

	printf ("List of lexems:\n");
	while ( cur != NULL ) {
		printf ("[%s]", cur->lexem);
		cur = cur->next;
	}
	printf ("\n");
}


/* Return number of delim or 
*/
int isdelim (int c)
{
	const char smb [] = "&|<>";
	int i = 0;

	while ( smb[i] != '\0' ) {
		if ( smb[i++] == c ) {
			return i;
		}
	}

	return 0;
}


ListElem * new_element (Buffer * buf)
{
	ListElem * new_elem = malloc (sizeof(ListElem));
	new_elem->lexem = malloc (buf->cnt);
	printf ("Now copy buffer: [%s].\n", buf->string);
	strcpy (new_elem->lexem, buf->string);
	new_elem->t_lex = LEX_WORD;
	new_elem->lenght = buf->cnt;
	new_elem->next = NULL;

	free_buffer (buf);
	init_buffer (buf);

	return new_elem;
}


/*
Function, which initializate list for future cmd.
*/
int init_list (List * list)
{
	printf ("Initalizating list...\n");
	list->ptr = NULL;
	printf ("List is ready for fill.\n");

	return 0;
}


/*
Function, which return 1 symbol from stdin.
*/
int get_symbol ()
{
	return getchar ();
}


/*
Function, which add to list one element
*/
int add_to_list (List * list, ListElem * list_elem)
{
	printf ("Add to list now.\n");

	if ( list->ptr == NULL ) {
		list->ptr = list_elem;
	}
	else {
		ListElem * cur = list->ptr;
		ListElem * prev;
		while ( cur != NULL ) {
			prev = cur;
			cur = cur->next;
		}
		prev->next = list_elem;
	}

	return 0;
}


/*
Function, which create full list of lexems
and return 0 (or lenght of list)
or return -1 if EOF.
*/
int fill_list (List * list)
{
	int c;			// symbol from input;
	int fill_result = 0; 	// return value;
	ListElem * list_elem = NULL;
	State CS = H;
	Buffer * buf = malloc (sizeof (Buffer));

	init_buffer (buf);

	while ( 1 ) {
		if ( (c = get_symbol ()) == EOF ) {
			fill_result = EOF;
			break;
		}
		if ( (list_elem = feed_symbol (c, &CS, buf)) != NULL ) {
			add_to_list (list, list_elem);
		}
	}

	c = ' '; 	// wanna write digit code of space
	if ( (list_elem = feed_symbol (c, &CS, buf)) != NULL ) {
		add_to_list (list, list_elem);
	}

	return fill_result;
}


/*
Function, which "eating" 1 symbol
and try create 1 lexem.
Return: 0 if lexem not ready.
	1 if lexem ready.
*/
ListElem * feed_symbol (int c, State * ptr_CS, Buffer * buf)
{
	ListElem * ptr_elem = NULL;

	ptr_elem = step (c, ptr_CS, buf);

	if ( ptr_elem != NULL ) {
		step (c, ptr_CS, buf);
	}

	return ptr_elem;
}


/*
Function, which take 1 step of lex. automate.
*/
ListElem * step (int c, State * ptr_CS, Buffer * buf)
{
	switch ( *ptr_CS ) {
	case H:
		return state_H (c, ptr_CS, buf);
	case WORD:
		return state_WORD (c, ptr_CS, buf);
	case DELIM:
		return state_DELIM (c, ptr_CS, buf);
	default:
		*ptr_CS = H;
		printf ("Error situation. Default branch in switch.\n");
		return 0;
	};
}


/*
state H
*/
ListElem * state_H (int c, State * CS, Buffer * buf)
{
	if ( isspace (c) ) {
		return NULL;
	}
	else if ( isdigit (c) || isalpha (c) ) {
		printf ("State changed: H->WORD.\n");
		add_symbol (c, buf);
		*CS = WORD;
		return NULL;
	}
	else if ( isdelim (c) ) {
		printf ("State changed: H->DELIM.\n");
		add_symbol (c, buf);
		*CS = DELIM;
		return NULL;
	}
	return 0;
};


/*
state WORD
*/
ListElem * state_WORD (int c, State * CS, Buffer * buf)
{
	if ( isdigit (c) || isalpha (c) ) {
		add_symbol (c, buf);
		return NULL;
	}
	else {
		printf ("State changed: WORD->H.\n");
		*CS = H;
		return new_element (buf);
	}
}


/*
state DELIM
*/
ListElem * state_DELIM (int c, State * CS, Buffer * buf)
{
	if ( isdelim (c) ) {
		add_symbol (c, buf);
		return NULL;
	}
	else {
		printf ("State changed: DELIM->H.\n");
		*CS = H;
		return new_element (buf);
	}
};

