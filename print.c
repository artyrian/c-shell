#include "print.h"


/* Print a list */
void print ( struct frame *primary)
{
	struct list * tmp = primary->first;
	while (tmp != NULL) {
		printf ("[%s]", primary->first->str );
		tmp = (primary->first)->next;
		primary->first = tmp;
	}
}




void printflg (struct checkers *flg)
{
	printf ("&=%d, R=%d, W=%d, A=%d. NumPipes=%d.\n",
	flg->bg,
	flg->read,
	flg->write,
	flg->append,
	flg->numPipe
	);
}

void PrintLng (struct all *g)
{
	while (g->first != NULL) {
		printf ("{ ");
		print (g->first->cmd);
		g->first = g->first->next;
		printf (" }\n");
	}
}


