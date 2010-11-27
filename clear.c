#include "clear.h"


/* Dispose list */
void FreeList (struct frame *primary)
{ 
	struct list * tmp;
	tmp = primary->first;
	while (tmp != NULL) {
		tmp = (primary->first)->next;
		free(primary->first->str);
		free(primary->first);
		primary->first = tmp;
	}
	free (primary);
}


void FreeLng (struct all **g)
{
	struct lng *tmp = (*g)->first;
	while (tmp != NULL) {
		tmp = (*g)->first->next;//-
		FreeList ((*g)->first->cmd);//+
		free ((*g)->first);//-
		(*g)->first = tmp;//-
	}
	free ((*g));//-
}


