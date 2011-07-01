#ifndef _EXEC_H_
#define _EXEC_H_

#include "parse.h"

char ** convert_list (List *);
int print_array (char **);
int free_array (char ** array);
int exec_list (char **);

#endif
