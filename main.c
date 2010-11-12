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
/* Structure of massive of pointers*/
struct masptr
{
	char *param;
	struct masptr *next;
};

/* Structure of flags (and maybe some ctrs) */
struct checkers
{
	int fExit;	// flag-signal of Exit
	int fError;	// flag-signal about error.
	int fMyCmd;	// flag-signal of use my own cmd.
	int fBg;	// flag-signal of turn on backgorund.
	int fEnter;	// flag-signal of pressed button "Enter".
	int fWordOut;	// flag-signal that u not in word.
	int fbslash;	// flag-signal last symbol os '\'.
	int fquote;	// flaf-signal u in the quotes.
	int fstick;	// flag-signal use conveer.
	int fRead;	// flag to Read from file and send to shell.
	int fWrite;	// flag to Write to file and send to shell.
	int fAppend;	// flag to Append to file and send to shell.
	int fSpec;
	int fNextSpec;
	int NumRead;
	int NumWrite;
};



struct file
{
	char *In;
	char *Out;
};

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




void SpecSymbTable(struct checkers *flg, struct frame *p, int c)
{
	switch(c){
		case '>': { 
			flg->fWrite = !OFF;		
			flg->NumWrite = p->count;
			break;
		}	
		case '<': {
			flg->fRead = !OFF;		
			flg->NumRead = p->count;
			break;
		}	
		case '&': {
			flg->fBg = !OFF;		
			break;
		}	
		case '|': {
			flg->fstick = !OFF;		
			break;
		}
	}
}



void NextSpecSymbTable(struct checkers *flg, struct frame *p, int c)
{
	switch(c){
		case '>':{ 
			flg->fAppend = !OFF;		
			flg->NumWrite = p->count;
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



/* At end of read each symbol do this */
void actionend(int *last_c, int c, struct checkers  *flg, struct frame **p)
{
	if (flg->fSpec){
		if (flg->fNextSpec)
			NextSpecSymbTable(flg, *p, c);	
		else 
			SpecSymbTable(flg, *p, c);
	}
	if (*last_c == '\\')
		*last_c = EOF;
	else
		*last_c = c;
	flg->fWordOut = OFF;
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



/* Dispose buffer */
void FreeBuf (struct buffer **buf)
{
	free ( (*buf)->str );
	free (*buf);
}



/* If Quote Enabled check c of EOF and Enter, then tell about result 
 */
int QuoteEnabled (int c)
{	
	int fErr;
	if ( c == '\n')
		printf ("> ");
	if ( c == EOF ) {
		printf ("\nExpected quotes!\n ");
		fErr = !OFF;
		return !OFF;
	}
	return OFF;
}



/* If BSlash disabled, check c of '"' and '\'
 * Result is changing flags (quotes or bslash) and fWordOut
 */
void BSlashOff (int c, struct checkers *flg)
{
	if (c == '"') {
		flg->fquote = ! flg->fquote;
		flg->fWordOut = !OFF;
	}
	if (c == '\\')
		flg->fbslash = flg->fWordOut = !OFF;
}



/* If NOT "In Word" - work WordOut() 
 */
void WordOut (struct frame *primary, struct buffer **buf,
                  char c, char cb, struct checkers *flg)
{
	if (c == '\n'){
		parse_word (primary, buf, cb);
		flg->fEnter = flg->fWordOut = !OFF;
	}
	if (( c == ' ') || (c == '\t')){
		parse_word (primary, buf, cb);
		flg->fWordOut = !OFF;
	}
	if (c == '>' || c == '<' ) {
		parse_word (primary, buf, cb);
		flg->fWordOut = !OFF;
	}
}



/* If "In Word" - work WordIn() */
void WordIn (struct buffer **buf, char c, struct checkers *flg)
{
	if ( flg->fBg == !OFF)
		flg->fError = !OFF;
	add_to_buf(buf, c);
	flg->fbslash = OFF;
}



/* If c == EOF then execute this fn.
 */
void cEOF (struct checkers *flg, int *c)
{
	flg->fExit = flg->fEnter = flg->fWordOut = !OFF;
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
	flg->fMyCmd = !OFF;
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
			flg->fMyCmd = changedir (path, ctr);
		if ( !strcmp (cmd, "exit") ){
			flg->fExit = cmdexit (flg);
			flg->fMyCmd = !OFF;
		}
	}
	free (tp);
}



void WordSpec (struct checkers *flg, struct frame *primary,
		int c, int cb)
{
	if ( flg->fSpec != OFF && (c == '>' || cb == '>'))
		flg->fNextSpec = !OFF;
	if ( c == '&' || c == '|' || c == '>' || c == '<'){
		flg->fSpec = !OFF;
		flg->fWordOut = !OFF;
	}
}


/* Diff. situations */
int ReadCommand(struct frame **primary, struct checkers **flg)
{
	struct buffer * buf; 
	int c , cb = ' ';		// cb - c before

	InitBuf(&buf);
	(*primary)->count = 0;
	(*primary)->first = (*primary)->last = NULL;

	while ( ( (*flg)->fEnter == OFF) && (c = getchar()) ){
		if ( c == EOF)//
			cEOF (*flg, &c);
		if ( (*flg)->fbslash == OFF )  //&
			BSlashOff (c, *flg);
		if ( !((*flg)->fbslash + (*flg)->fquote) )
			WordSpec (*flg, *primary, c, cb);
		if ( !((*flg)->fbslash+(*flg)->fquote)) //&
			WordOut(*primary, &buf,	c, cb, *flg);
		if ( (*flg)->fWordOut == OFF)
			WordIn(&buf, c, *flg);
		if ( (*flg)->fquote == !OFF )
			(*flg)->fError = QuoteEnabled (c);
		actionend(&cb, c, *flg, primary);
	}
	MyCmd(*primary, *flg);
	FreeBuf (&buf);	
	return (*flg)->fExit;
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
	close(fd);
	free (f);
}


void WriteToFile (struct frame *primary, struct checkers *flg)
{
	int save1, fd;
	struct file *f = (struct file *)malloc(sizeof(struct file));

	fflush(stdout);
	save1 = dup (1);

	f->Out = primary->first->str;
	if (!flg->fAppend)	
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

	if (flg->NumRead != -1)
		fUse--;
	if (flg->NumWrite != -1)
		fUse--;
	return fUse;
}


/* Function for create massive of arguments */
void Createmas(struct frame *primary, struct checkers *flg, char ***marg)
{
	int i = 0, j = 0;
	struct file *f = (struct file*)malloc(sizeof(struct file));
	(*marg) = (char**)malloc((primary->count+Redir(flg))*sizeof(char*));
	while (primary->first != NULL) {
		if ( flg->NumRead == j )
			ReadFromFile (primary, flg);	
		else 
			if ( flg->NumWrite == j )
				WriteToFile (primary, flg);
			else
				(*marg)[i++] = primary->first->str;
		primary->first = primary->first->next;
		j++;
	}
	(*marg)[i] = NULL;
	free (f);
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
void Callout (struct frame *primary, struct checkers **flg)
{
	int pid, status;

	if ( !(*flg)->fMyCmd) {	
		if ( !(pid = fork() ) )
			execcmd (primary, *flg);
		if ( !(*flg)->fBg )
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



void InitFlags(struct checkers **flg)
{
	(*flg)->fExit = OFF;
	(*flg)->fError = OFF;
	(*flg)->fMyCmd = OFF;
	(*flg)->fBg = OFF;
	(*flg)->fEnter = OFF;
	(*flg)->fWordOut = OFF;
	(*flg)->fbslash = OFF;
	(*flg)->fquote = OFF;
	(*flg)->fstick = OFF;
	(*flg)->fRead = OFF;
	(*flg)->fWrite = OFF;
	(*flg)->fAppend = OFF;
	(*flg)->fSpec = OFF;
	(*flg)->fNextSpec = OFF;
	(*flg)->NumRead = -1;
	(*flg)->NumWrite = -1;
}



void printflg (struct checkers *flg)
{
	printf ("|=%d, R=%d, W=%d, A=%d, NR=%d, NW=%d\n",
	flg->fstick,
	flg->fRead,
	flg->fWrite,
	flg->fAppend,
	flg->NumRead,
	flg->NumWrite);
}




/* Main function */
int main()
{
	struct frame *primary; 
	struct checkers *flg;
	flg = (struct checkers*)malloc(sizeof(struct checkers));
	flg->fExit = OFF;
	while ( flg->fExit == OFF ) {
		InitFlags (&flg);
		primary = (struct frame *)malloc(sizeof(struct frame));
		printf ("%s @ ", getcwd(NULL, 0) );
		flg->fExit = ReadCommand (&primary, &flg);
		if ( flg->fError == OFF )
			Callout(primary, &flg);
		else
			printf ("Error execute! Incorrect cmd.\n");
		waitzombie ();
		FreeList (primary);
		printf ("\n");
	}
	free (flg);
	return 0;
}
