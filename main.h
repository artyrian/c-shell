#ifndef _MAIN_H_
#define _MAIN_H_

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
	char * fileRead;
	char * fileWrite;
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
	int fr;
	int fw;
};

#endif
