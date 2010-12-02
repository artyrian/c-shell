#include "main.h"
#include "parse.h"
#include "print.h"
#include "clear.h"

/*Ask all sun's processes and say if founded zombie */
void waitzombie ()
{
	int pid, status;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
		printf("pid=%d, status=%d\n", pid, status);
}


/* Function for create massive of arguments */
void Createmas(struct all *g, struct frame *primary, struct checkers *flg, char ***marg)
{ int i = 0; (*marg) = (char**)malloc(primary->count*sizeof(char*));
	while (primary->first != NULL) {
		(*marg)[i++] = primary->first->str;
		primary->first = primary->first->next;
	}
	(*marg)[i] = NULL;
}


void ReadFD (struct all *g, struct checkers *flg)
{
	int fd;
	if ( flg->read != 0 )
	{
		fd = open (g->fileRead, O_RDONLY);
		if ( fd == -1){
			printf ("error(read from file)!\n");
			exit (1);
		}
		dup2(fd, 0);
		close(fd);
	}
}


void WriteFD(struct all *g, struct checkers *flg)
{
	int fd;

	if ( flg->write) 
	{
		if (!flg->append)	
			fd = open (g->fileWrite, O_CREAT|O_WRONLY|O_TRUNC, 0666);
		else	
			fd = open (g->fileWrite, O_CREAT|O_WRONLY|O_APPEND, 0666);
		if ( fd == -1){
			printf ("error(write to file)!\n");
			exit (1);
		}
		dup2(fd, 1);
		close(fd);
	}
}


/* Try to execute cmd */
void execcmd (struct all *g, struct frame *primary,
		struct checkers *flg, int i)
{
	char ** marg = NULL;
	char * cmd;
	if ( i == 0 )
		ReadFD (g, flg);
	if ( i == flg->numPipe )
		WriteFD (g, flg);
	Createmas(g, primary, flg, &marg);
	cmd = *marg;
	execvp (cmd, marg);
	printf ("%s\n", strerror ( errno ) );
	exit(1);
}


void StartPipe (struct all *g, struct checkers *flg)
{
	int i, j;
	int pid[flg->numPipe+1][2];
	int fd[flg->numPipe][2];
	struct lng * t_g;
	int tmp;
	
	t_g = g->first;

 	i = 0;
	pipe (fd[i]);
	if ( !(pid[i][0] = fork()) )
	{
		dup2 ( fd[i][1], 1);
		close (fd[i][1]);
		close (fd[i][0]);
		execcmd (g, g->first->cmd, flg, i);
	}

	i++;
	g->first = g->first->next;

	for ( i = 1; i < flg->numPipe; i++ )
	{
		pipe (fd[i]);
		if ( !(pid[i][0] = fork ()) ) 
		{
			dup2 ( fd[i][1], 1);
			close (fd[i][1]);
			close (fd[i][0]);
			close (fd[i-1][1]);
			
			dup2 ( fd[i-1][0], 0);
			close (fd[i-1][0]);
			close (fd[i-1][1]);
			close (fd[i][0]);

			execcmd (g, g->first->cmd, flg, i);
		}
		g->first = g->first->next;
		close (fd[i-1][0]);
		close (fd[i-1][1]);
	}

	if ( !(pid[i][0] = fork()) )
	{
		dup2 ( fd[i-1][0], 0);
		close (fd[i-1][0]);
		close (fd[i-1][1]);
		execcmd (g, g->first->cmd, flg, i);
	}
	close (fd[i-1][0]);
	close (fd[i-1][1]);

	if ( flg->bg == 0 )
	{

		j = 0;
		while ( flg->numPipe +1 != 0 )
		{ 
			tmp = waitpid (-1, &pid[i][1], WNOHANG);
			for ( i = 0; i <= flg->numPipe; i++)
			{
				if ( tmp == pid[i][0] )
				{
					pid[i][0] = pid[flg->numPipe][0];
					flg->numPipe--;
					j++;
				}
			}
		} 
	}
	g->first = t_g;
}
	

void StartOne (struct all *g, struct checkers * flg)
{
	int pid, status;
	if ( !(pid = fork()) )
	{
		execcmd (g, g->first->cmd, flg, 0);
	}

	if ( !flg->bg)
		waitpid (pid, &status, 0);
}


void StartCmd (struct all *g, struct checkers *flg)
{
	if ( !(flg)->myCmd)
	{
		if ( flg->numPipe )
			StartPipe (g, flg);	
		else
			StartOne (g,flg);
	}
}




/* Main function */
int main()
{
	struct checkers *flg;
	struct all *general;

	flg = (struct checkers*)malloc(sizeof(struct checkers));
	flg->exit = OFF;
	while ( flg->exit == OFF ) {
		general = (struct all*)malloc(sizeof(struct all));
		general->fileRead = general->fileWrite = NULL;
		printf ("\n@");
		ReadCommand (general, flg);
		if ( flg->error == OFF ){
			StartCmd (general, flg);
		}
	//	printflg (flg);
	//	PrintLng( general);
	//	printf ("g->fileRead=%s.\n", general->fileRead);
	//	printf ("g->fileWrite=%s.\n", general->fileWrite);
		FreeLng (&general);
		waitzombie ();
	}
	free (flg);
	return 0;
}
