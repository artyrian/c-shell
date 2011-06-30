#include <stdio.h>
#include <stdlib.h>

#include "parse.h"



int main ()
{
	List * lexem_list = malloc (sizeof (List));
	int fill_result = 0;

	do {
		init_list (lexem_list);
		fill_result = fill_list (lexem_list);
	} while ( fill_result != EOF );

	print_list (lexem_list);

	return 0;
}
