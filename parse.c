#include "parse.h"

void InitFlags(struct checkers *flg)
{
	flg->exit = OFF;
	flg->error = OFF;
	flg->myCmd = OFF;
	flg->bg = OFF;
	flg->enter = OFF;
	flg->wordOut = OFF;
	flg->bslash = OFF;
	flg->quote = OFF;
	flg->stick = OFF;
	flg->read = OFF;
	flg->write = OFF;
	flg->append = OFF;
	flg->spec = OFF;
	flg->nextSpec = OFF;
	flg->numRead = -1;
	flg->numWrite = -1;
	flg->numPipe = 0;
	flg->fr = OFF;
	flg->fw = OFF;
}



void AddWord (struct frame *primary, struct buffer **buf)
{
	struct list *tmp;
	char *str;

	tmp = (struct list *) malloc(sizeof(struct list));
	str = (char *) malloc( ((*buf)->count + 1)*sizeof(char) );
	strcpy (str, (*buf)->str);
	tmp->str = str;
	tmp->next = NULL;
	if (primary->first == NULL)
		primary->first = tmp;
	else 
		primary->last->next = tmp;
	primary->last = tmp;

}



/* Extend buffer (linear) */
void extend_buffer (struct buffer ** buf)
{
	char * tmp_buf;

	tmp_buf = malloc (((++(*buf)->part) * PART_SIZE) * sizeof(char));
	strcpy (tmp_buf, (*buf)->str);
	free ((*buf)->str);
	(*buf)->str = tmp_buf;
}



/* Add new symbol to buffer and allocate buf, if it's need */
void add_to_buf (struct buffer ** buf, char c )
{	
	if ( ((*buf)->part)*PART_SIZE  == ((*buf)->count) + 1)
		extend_buffer (buf);
	(*buf)->str[(*buf)->count] = c;
	(*buf)->str[(*buf)->count + 1] = '\0';
	(*buf)->count++;
}


void SafeNameFile (struct all *g, struct buffer *buf, struct checkers *flg)
{
	char *str;

	str = (char *) malloc( (buf->count + 1)*sizeof(char) );
	strcpy (str, buf->str);
	if (flg->numRead != -1)
	{
		g->fileRead = str;
		flg->numRead = -1;
	}

	if (flg->numWrite != -1)
	{
		g->fileWrite = str;
		flg->numWrite = -1;
	}
	buf->count = 0;
	buf->part = 1;

}

/* Add word to list if it word. Check to first || non-first elem of list
 */
void parse_word(struct frame *primary, struct buffer **buf, int cb,
		struct checkers *flg, struct all *g)
{
	if ( (cb != ' ') && (cb != '\t')  && ((*buf)->count > 0) )
	{		
		if ( flg->numRead != -1 && !flg->fr )
		{
			SafeNameFile (g, *buf, flg);
			flg->fr = !OFF;
		}
		else
			if (flg->numWrite != -1 && !flg->fw )
			{
				SafeNameFile (g, *buf, flg);
				flg->fw = !OFF;
			}
			else
			{
				AddWord (primary, buf);
				(*buf)->count = 0;
				(*buf)->part = 1;
				primary->count++;
			}
	
	}
}



/* Initialization structure of buffer */
void InitBuf(struct buffer ** buf)
{
	*buf = (struct buffer *) malloc (sizeof(struct buffer));
	(*buf)->str = (char *) malloc(PART_SIZE*sizeof(char));
	(*buf)->count = 0;
	(*buf)->part = 1;
}


void InitPrimary (struct all *g)
{
	g->first = (struct lng*)malloc(sizeof(struct lng));
	g->last = (struct lng*)malloc(sizeof(struct lng));
	g->first->cmd = (struct frame*)malloc (sizeof(struct frame));
	g->first->cmd->count = 0;
	g->first->cmd->first = g->first->cmd->last = NULL;
	g->first->next = NULL;
	g->last = g->first;
}


/* Dispose buffer */
void FreeBuf (struct buffer **buf)
{
	free ( (*buf)->str );
	free (*buf);
}





void SpecSymbTable(struct checkers *flg, struct frame *p, int c)
{
	switch(c){
		case '>': { 
			flg->write = !OFF;		
			flg->numWrite = p->count;
			break;
		}	
		case '<': {
			flg->read = !OFF;		
			flg->numRead = p->count;
			break;
		}	
		case '&': {
			flg->bg = !OFF;		
			break;
		}	
		case '|': {
			flg->stick = !OFF;		
			break;
		}
	}
}



void NextSpecSymbTable(struct checkers *flg, struct frame *p, int c)
{
	switch(c){
		case '>':{ 
			if (flg->write)
				flg->append = !OFF;		
			else
				flg->write = !OFF;
			flg->numWrite = p->count;
			break;
		}
		case '&':{ 
			//flg->fBg = !OFF;		
			break;
		}
		case '|':{ 
			//flg->fstick = !OFF;		
			break;
		}
	}
}


void CreateRing (struct all *g, struct checkers *flg)
{
	struct lng *tmp = (struct lng*)malloc(sizeof(struct lng));	
	tmp->cmd = (struct frame*)malloc (sizeof(struct frame));
	tmp->cmd->count = 0;
	tmp->cmd->first = tmp->cmd->last = NULL;
	tmp->next = NULL;
	g->last->next = tmp;  
	g->last = tmp;

	flg->stick = OFF;
	flg->numPipe ++;
}


/* At end of read each symbol do this */
void ActionEnd(int *last_c, int c,
		struct checkers  *flg,
		struct all *g,
		struct buffer **buf)
{	
	if (flg->spec){
		if (flg->nextSpec)
			NextSpecSymbTable(flg, g->first->cmd, c);	
		else 
			SpecSymbTable(flg, g->first->cmd, c);
	}
	if (*last_c == '\\')
		*last_c = EOF;
	else
		*last_c = c;

	if ( flg->stick ){
		CreateRing (g, flg);	
		FreeBuf (buf);
		InitBuf (buf);
	}
	flg->wordOut = OFF;
}





/* If Quote Enabled check c of EOF and Enter, then tell about result 
 */
void QuoteEnabled (int c, struct checkers *flg)
{	
	if ( c == '\n')
		printf ("> ");
	if ( c == EOF ) {
		printf ("\nExpected quotes!\n ");
		flg->error = !OFF;
	}
}



/* If BSlash disabled, check c of '"' and '\'
 * Result is changing flags (quotes or bslash) and fWordOut
 */
void BSlashOff (int c, struct checkers *flg)
{
	if (c == '"') {
		flg->quote = ! flg->quote;
		flg->wordOut = !OFF;
	}
	if (c == '\\')
		flg->bslash = flg->wordOut = !OFF;
}



/* If NOT "In Word" - work WordOut() 
 */
void WordOut (struct frame *primary, struct buffer **buf,
                  char c, char cb, struct checkers *flg, struct all *g)
{
	if (c == '\n'){
		parse_word (primary, buf, cb, flg, g);
		flg->enter = flg->wordOut = !OFF;
	}
	if (( c == ' ') || (c == '\t')){
		parse_word (primary, buf, cb, flg, g);
		flg->wordOut = !OFF;
	}
	if (c == '>' || c == '<' || c == '|') {
		parse_word (primary, buf, cb, flg, g);
		flg->wordOut = !OFF;
	}
}



/* If "In Word" - work WordIn() */
void WordIn (struct buffer **buf, char c, struct checkers *flg)
{
	if ( flg->bg == !OFF)
		flg->error = !OFF;
	add_to_buf(buf, c);
	flg->bslash = OFF;
}



/* If c == EOF then execute this fn.
 */
void cEOF (struct checkers *flg, int *c)
{
	flg->exit = flg->enter = flg->wordOut = !OFF;
	*c = ' ';
	printf ("\n");
}



/* Change directory to *path if it possible or say about it.
 */
void dirpath(char *path)
{
	if ( chdir (path) )
		printf ("Can't find %s\n", path);
}


/* Change directory to home if it possible or say about it.
 */
void dirhome(char *path)
{
	if ( chdir(getenv("HOME")) )
		printf ("Can't find home directory\n");	
}


/* Cmd of "cd". Use system call CHDIR.
 * Return flag that used my own cmd.
 */
int changedir(char *path, int ctr)
{
	if ( ctr == 1 )
		dirhome (path);
	else
		dirpath (path);
	return !OFF;
}


/* Cmd of "exit". Turn on the flag,
 * Return flag that used my own cmd.
 */
int cmdexit(struct checkers *flg)
{	
	flg->myCmd = !OFF;
	return !OFF;
}



/* Check for my own commands in list */
void MyCmd (struct frame *primary, struct checkers *flg)
{
	struct frame *tp = (struct frame *)malloc(sizeof(struct frame));
	char *cmd, *path;
	int ctr = primary->count;

	if (ctr > 0){
		cmd = primary->first->str;
		if  (ctr > 1){
			tp->first = primary->first->next;
			path = tp->first->str;
		}
		if ( !strcmp (cmd, "cd") )
			flg->myCmd = changedir (path, ctr);
		if ( !strcmp (cmd, "exit") ){
			flg->exit = cmdexit (flg);
			flg->myCmd = !OFF;
		}
	}
	free (tp);
}



void WordSpec (struct checkers *flg, struct frame *primary,
		int c, int cb)
{
	if ( flg->spec != OFF && (c == '>' || cb == '>'))
		flg->nextSpec = !OFF;
	if ( c == '&' || c == '|' || c == '>' || c == '<'){
		flg->spec = !OFF;
		flg->wordOut = !OFF;
	}
}



/* Diff. situations */
void ReadCommand(struct all *g, struct checkers *flg)
{
	struct buffer * buf; 
	int c , cb = ' ';		// cb - c before

	InitFlags (flg);
	InitBuf(&buf);
	InitPrimary(g);

	while ( ( (flg)->enter == OFF) && (c = getchar()) ){
		if ( c == EOF)
			cEOF (flg, &c);
		if ( flg->bslash == OFF ) 
			BSlashOff (c, flg);
		if ( !(flg->bslash + flg->quote) )
			WordSpec (flg, g->first->cmd, c, cb);
		if ( !(flg->bslash+flg->quote))
			WordOut(g->last->cmd, &buf, c, cb, flg, g);
		if ( flg->wordOut == OFF)
			WordIn(&buf, c, flg);
		if ( flg->quote == !OFF )
			QuoteEnabled (c, flg);
		ActionEnd (&cb, c, flg, g, &buf);
		if ( ( c == '&') && !(flg->quote) )
			flg->bg = !OFF;
	}
	MyCmd(g->first->cmd, flg);
	FreeBuf (&buf);	
}


