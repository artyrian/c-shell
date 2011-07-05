#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


void print_list (List *);
int isword (int);
int isdelim (int);
ListWord * new_element (Buffer * buf);


void print_list (List * list)
{
	ListWord * cur = list->cmd;

	printf ("List of lexems:\n");
	while ( cur != NULL ) {
		printf ("[%s]", cur->lexem);
		cur = cur->next;
	}
	printf ("\n");
}


int isword (int c)
{
	if ( isalpha (c) || isdigit (c) || c == '-' || c == '.' ) {
		return 1;
	}
	else {
		return 0;
	}
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


ListWord * new_element (Buffer * buf)
{
	ListWord * new_elem = malloc (sizeof(ListWord));
	new_elem->lexem = malloc (buf->cnt);
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
	list->cmd = NULL;
	list->count = 0;
	list->next = NULL;

	return 0;
}


/*
Function< which destroy list of lexems.
*/
int free_list (List * list)
{
	ListWord * cur = list->cmd;
	ListWord * prev;

	while ( cur != NULL ) {
		prev = cur;
		cur = cur->next;
		free (prev->lexem);
		free (prev);
	}

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
int add_to_list (List * list, ListWord * list_elem)
{
	if ( list->cmd == NULL ) {
		list->cmd = list_elem;
	}
	else {
		ListWord * cur = list->cmd;
		ListWord * prev;
		while ( cur != NULL ) {
			prev = cur;
			cur = cur->next;
		}
		prev->next = list_elem;
	}
	++ list->count;

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
	ListWord * list_elem = NULL;
	State CS = H;
	Buffer * buf = malloc (sizeof (Buffer));

	init_buffer (buf);

	while ( 1 ) {
		c = get_symbol ();
		if ( (c == EOF) || c == '\n' ) {
			fill_result = c;
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
ListWord * feed_symbol (int c, State * ptr_CS, Buffer * buf)
{
	ListWord * ptr_elem = NULL;

	ptr_elem = step (c, ptr_CS, buf);

	if ( ptr_elem != NULL ) {
		step (c, ptr_CS, buf);
	}

	return ptr_elem;
}


/*
Function, which take 1 step of lex. automate.
*/
ListWord * step (int c, State * ptr_CS, Buffer * buf)
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
ListWord * state_H (int c, State * CS, Buffer * buf)
{
	if ( isspace (c) || c == EOF ) {
		return NULL;
	}
	else if ( isword (c) ) {
		add_symbol (c, buf);
		*CS = WORD;
		return NULL;
	}
	else if ( isdelim (c) ) {
		add_symbol (c, buf);
		*CS = DELIM;
		return NULL;
	}
	else if ( c == '"' ) {
		*CS = QUOTE;
		return NULL;
	}
	return 0;
};


/*
state WORD
*/
ListWord * state_WORD (int c, State * CS, Buffer * buf)
{
	if ( isword (c) ) {
		add_symbol (c, buf);
		return NULL;
	}
	else {
		*CS = H;
		return new_element (buf);
	}
}


/*
state DELIM
*/
ListWord * state_DELIM (int c, State * CS, Buffer * buf)
{
	if ( isdelim (c) ) {
		add_symbol (c, buf);
		return NULL;
	}
	else {
		*CS = H;
		return new_element (buf);
	}
};


/*
state QUOTE
*/
ListWord * state_QUOTE (int c, State * CS, Buffer * buf)
{
	if ( c != '"' ) {
		add_symbol (c, buf);
		return NULL;
	}
	else {
		*CS = H;
		return new_element (buf);
	}
};
