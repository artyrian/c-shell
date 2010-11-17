#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define OFF 0
#define PART_SIZE 128
#define COMMAND_EXIT "exit"
#define VERSION_NUMBER 1

/* Structure of list */
struct list
{
	char *str;
	struct list *next;
};

/* Structure of buffer */
struct buffer
{
	char *str;
	int count;
	int part;
};

/* Structure of pointer to list */
struct frame
{
	int count;
	struct list *first;
	struct list *last;
};

struct lng
{
	struct frame *cmd;
	struct lng *next;
};

struct all
{
	struct lng *first;
	struct lng *last;
};
/* Structure of massive of pointers*/
struct masptr
{
	char *param;
	struct masptr *next;
};

/* Structure of flags (and maybe some ctrs) */
struct checkers
{
	int exit;	// flag-signal of Exit
	int error;	// flag-signal about error.
	int myCmd;	// flag-signal of use my own cmd.
	int bg;		// flag-signal of turn on backgorund.
	int enter;	// flag-signal of pressed button "Enter".
	int wordOut;	// flag-signal that u not in word.
	int bslash;	// flag-signal last symbol os '\'.
	int quote;	// flaf-signal u in the quotes.
	int stick;	// flag-signal use conveer.
	int read;	// flag to Read from file and send to shell.
	int write;	// flag to Write to file and send to shell.
	int append;	// flag to Append to file and send to shell.
	int spec;
	int nextSpec;
	int numRead;
	int numWrite;
	int numPipe;
};



struct file
{
	char *In;
	char *Out;
};



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

/* Add word to list if it word. Check to first || non-first elem of list
 */
void parse_word(struct frame *primary, struct buffer **buf, int cb)
{
	if ( (cb != ' ') && (cb != '\t')  && ((*buf)->count > 0) ){
		AddWord (primary, buf);
		(*buf)->count = 0;
		(*buf)->part = 1;
		primary->count++;
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
			flg->append = !OFF;		
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




/* Print a list */
void print ( struct frame *primary)
{
	struct list * tmp = primary->first;
	while (tmp != NULL) {
		printf ("[%s]\n", primary->first->str );
		tmp = (primary->first)->next;
		primary->first = tmp;
	}
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
                  char c, char cb, struct checkers *flg)
{
	if (c == '\n'){
		parse_word (primary, buf, cb);
		flg->enter = flg->wordOut = !OFF;
	}
	if (( c == ' ') || (c == '\t')){
		parse_word (primary, buf, cb);
		flg->wordOut = !OFF;
	}
	if (c == '>' || c == '<' || c == '|') {
		parse_word (primary, buf, cb);
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
			WordOut(g->last->cmd, &buf, c, cb, flg);
		if ( flg->wordOut == OFF)
			WordIn(&buf, c, flg);
		if ( flg->quote == !OFF )
			QuoteEnabled (c, flg);
		ActionEnd (&cb, c, flg, g, &buf);
	}
	MyCmd(g->first->cmd, flg);
	FreeBuf (&buf);	
}




void ReadFromFile (struct frame *primary, struct checkers *flg)
{
	int save1, fd;
	struct file *f = (struct file *)malloc(sizeof(struct file));

	fflush(stdin);
	save1 = dup (0);

	f->In = primary->first->str;
	fd = open (f->In, O_RDONLY);
	if ( fd == -1){
		printf ("ERROR(Read from file)!\n");
		exit (1);
	}
	dup2(fd, 0);
	close(fd); free (f); }


void WriteToFile (struct frame *primary, struct checkers *flg)
{
	int save1, fd;
	struct file *f = (struct file *)malloc(sizeof(struct file));

	fflush(stdout);
	save1 = dup (1);

	f->Out = primary->first->str;
	if (!flg->append)	
		fd = open (f->Out, O_CREAT|O_WRONLY|O_TRUNC, 0666);
	else	
		fd = open (f->Out, O_CREAT|O_WRONLY|O_APPEND, 0666);
	if ( fd == -1){
		printf ("ERROR(Write to file)!\n");
		exit (1);
	}
	dup2(fd, 1);
	close(fd);
	free (f);
}



int Redir(struct checkers *flg)
{
	int fUse = 0;

	if (flg->numRead != -1)
		fUse--;
	if (flg->numWrite != -1)
		fUse--;
	return fUse;
}


/* Function for create massive of arguments */
void Createmas(struct frame *primary, struct checkers *flg, char ***marg)
{
	int i = 0, j = 0;
	(*marg) = (char**)malloc((primary->count+Redir(flg))*sizeof(char*));
	while (primary->first != NULL) {
		if ( flg->numRead == j )
			ReadFromFile (primary, flg);	
		else 
			if ( flg->numWrite == j )
				WriteToFile (primary, flg);
			else
				(*marg)[i++] = primary->first->str;
		primary->first = primary->first->next;
		j++;
	}
	(*marg)[i] = NULL;
}



/* Try to execute cmd */
void execcmd (struct frame *primary, struct checkers *flg)
{
	char ** marg = NULL;
	char * cmd;

	Createmas(primary, flg, &marg);
	cmd = *marg;
	execvp (cmd, marg);
	printf ("%s\n", strerror ( errno ) );
	exit(1);
}



/* Call out programms */
void Callout (struct lng *r, struct checkers *flg)
{
	int pid, status;

	if ( !(flg)->myCmd) {	
		if ( !(pid = fork() ) )
			execcmd (r->cmd, flg);
		if ( !(flg)->bg )
			waitpid (pid, &status, 0);
	}
}



/*Ask all sun's processes and say if founded zombie */
void waitzombie ()
{
	int pid, status;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
		printf("pid=%d, status=%d\n", pid, status);
}




void printflg (struct checkers *flg)
{
	printf ("|=%d, R=%d, W=%d, A=%d, NR=%d, NW=%d\n",
	flg->stick,
	flg->read,
	flg->write,
	flg->append,
	flg->numRead,
	flg->numWrite);
}

void PrintLng (struct all *g)
{
	while (g->first != NULL) {
		printf ("{\n");
		print (g->first->cmd);
		g->first = g->first->next;
		printf ("}\n");
	}
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


void StartPipe (struct all *g, struct checkers *flg)
{
	int i, pid, status, fd[flg->numPipe][2];

	for ( i = 0; i < flg->numPipe+1; i++ ){
		if ( i != flg->numPipe )
			pipe (fd[i]);
		if ( !(pid = fork()) ) {
			if ( i )
				dup2 (fd[i-1][0],0);
			if ( i != flg->numPipe )
				dup2 ( fd[i][1], 1);
			execcmd (g->first->cmd, flg);
		} else {
			if ( i != flg->numPipe){
				close ( fd [i][1] );
			}
				waitpid (pid, &status, 0);
		}
		g->first = g->first->next;
	}

}

void StartCmd (struct all *g, struct checkers *flg)
{
	int pid, status;

	if ( !(flg)->myCmd)
		if ( !(pid = fork() ) )
		{
			StartPipe (g, flg);	
			exit (1);
		}
	if ( !(flg)->bg )
		waitpid (pid, &status, 0);
}




/* Main function */
int main()
{
	struct checkers *flg;
	struct all *general;
	char *cwd = getcwd(NULL, 0);

	flg = (struct checkers*)malloc(sizeof(struct checkers));
	flg->exit = OFF;
	while ( flg->exit == OFF ) {
		general = (struct all*)malloc(sizeof(struct all));
		printf ("\n%s @ ", cwd);
		ReadCommand (general, flg);
		if ( flg->error == OFF ){
			StartCmd (general, flg);
		}
		FreeLng (&general);
		waitzombie ();
	}
	free (cwd);
	free (flg);
	return 0;
}
