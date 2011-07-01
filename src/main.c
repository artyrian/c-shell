#include <stdio.h>
#include <stdlib.h>

#include "parse.h"
#include "exec.h"



int main ()
{
	List * lexem_list = malloc (sizeof (List));
	char ** lexem_array = NULL;
	int fill_result = 0;

	do {
		init_list (lexem_list);
		fill_result = fill_list (lexem_list);
		lexem_array = convert_list (lexem_list);
		print_array (lexem_array);
		exec_list (lexem_array);
		free_list (lexem_list);
		free_array (lexem_array);
	} while ( fill_result != EOF );

	
	free (lexem_list);
	return 0;
}
