#include "exec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


/*
Fucntion, which create from list  a array of array ptrs.
*/
char ** convert_list (List * list)
{
	ListElem * cur = list->cmd;
	char ** array = malloc ( (list->count + 1) * sizeof (char *));
	int i = 0;

	printf ("Convert list to array.\n");

	while ( cur != NULL ) {
		if ( cur->t_lex == LEX_WORD ) {
			array [i++] = cur->lexem;
		}
		else if ( cur->t_lex == LEX_READ ) {

		}
		else if ( cur->t_lex == LEX_WRITE ) {

		}
		else if ( cur->t_lex == LEX_APPEND ) {

		}
		else if ( cur->t_lex == LEX_PIPE ) {

		}
		cur = cur->next;
	}
	array [i] = NULL;

	return array;
}


/*
Function< which print array of lexems.
*/
int print_array (char ** array)
{
	int i;

	printf ("Print lexem's array:\n");
	for (i = 0; array [i] != NULL; ++i) {
		printf ("[%s]", array [i]);
	}
	printf ("\n");

	return 0;
}


/*
Function, which erase array
*/
int free_array (char ** array)
{
	free (array);
	return 0;
}


/*
Function, whicb exeCUTE cmd.
*/
int exec_list (char ** array)
{
	int pid;
	int status;

	if ( (pid = fork ()) == 0 ) {
		execvp (array[0], array);	
		printf ("Error in executing:%s\n", strerror (errno) );
		exit (1);
	}
	else {
		waitpid (pid, &status, 0);
	}

	return 0;
}
