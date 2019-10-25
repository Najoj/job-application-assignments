/**
 * Written by Johan Öhlin, 2016
 * johanohlin@gmx.com
 *
 * Fetches all running processes, waits a little, fetch all running processes and
 * print which ones are gone and which ones have been created. Only considering
 * PID, not respecting COMM (i.e. new process with with old PID is old.)
 *
 * Compile with -lprocps
 */

#include <proc/readproc.h>	/* Library to read /proc */
#include <stdio.h>		/* Standard IO. printf */
#include <stdlib.h>		/* Standard library. malloc and exit status */
#include <string.h>		/* Manage strings. memset and strcpy */

/* Wait time between fetches. */
#define TIME 60

/* Linked list with process name and its ID */
typedef struct process {
	char comm[256];		/* process name */
	int pid;		/* process id */
	struct process *next;	/* pointer to next process */
} PROCESS;

/**
 * Here is where the magic happens. Fetch PID and COMM for all running processes,
 * and put them into a linked list.
 * Returns the number of processes found. Mostly useful while debugging.
 */
int fetch_processes(PROCESS * list)
{
	/* Counter so see how many processes are found. */
	int n = 0;

	/* PROCTAB is a struct defined in readproc.h. Has info that openproc needs. */
	PROCTAB *proc;
	/* Contain the proc info. Also defined in readproc.h */
	proc_t proc_info;

	/* Set proc_info settings to zero. */
	memset(&proc_info, 0, sizeof(proc_info));
	/* Open proc for reading. Flags needed to read status files in /proc */
	proc = openproc(PROC_FILLSTATUS | PROC_FILLSTATUS);

	/* As long as there is something more to read. */
	while (readproc(proc, &proc_info) != NULL) {
		/* Increment counter to see how many processes were found */
		n++;

		/* Copy process name. */
		strcpy(list->comm, proc_info.cmd);
		/* Task ID, which is the process ID. */
		list->pid = proc_info.tid;

		/* Allocate next process */
		list->next = malloc(sizeof(PROCESS));
		/* Go to next process */
		list = list->next;
	}

	/* Ensures that last element in list is NULL */
	list = NULL;
	/* Closing the open proc */
	closeproc(proc);

	return n;
}

/* Prints a list of processes. Has to terminate with NULL. */
void print_list(PROCESS * list, int length)
{
	printf("PID\t\tCOMM\n");
	/* NULL is at the end of the list */
	while (NULL != list) {
		/* Print info and get next process in list */
		printf("%d\t%s\n", list->pid, list->comm);
		list = list->next;
	}
	return;
}

/* Prints the difference between two lists. */
void print_diff(PROCESS * first, PROCESS * second)
{
	/* Boolean to see whether there have been a change at all */
	int nodiff = 1;
	/* As long as we have not read everything from the two lists */
	while (NULL != first || NULL != second) {
		/* If first list is NULL, it is done and thus the rest are new. */
		if (first == NULL || first->pid > second->pid) {
			printf("new:\t%d - %s\n", second->pid, second->comm);
			second = second->next;
			nodiff = 0;
		}
		/* If second list is NULL, it is has reach end of list and thus the rest are old. */
		else if (second == NULL || first->pid < second->pid) {
			printf("gone:\t%d - %s\n", first->pid, first->comm);
			first = first->next;
			nodiff = 0;
		}
		/* Print nothing when there is no difference. */
		else if (first->pid == second->pid) {
			/*printf("stay:\t%d - %s\n", first->pid, first->comm); */
			/* Only skip */
			first = first->next;
			second = second->next;
		}
	}
	/* If there has not been a change at all */
	if (nodiff) {
		printf("No changes in process lists\n");
	}
	return;
}

/* Walk through the linked list to befree them */
void free_list(PROCESS * list)
{
	/* Temporary var to keep next process of the list. */
	PROCESS *next;
	while (NULL != list) {
		/* Save next list element, so that we won't lose it */
		next = list->next;
		free(list);
		list = next;
	}
	return;
}

int main(argc, argv)
int argc;
const char *argv[];
{
	/* Allocates the first element of the lists. */
	PROCESS *before = malloc(sizeof(PROCESS));
	PROCESS *after = malloc(sizeof(PROCESS));

	/* Interesting, but mostly used for debugging. */
	int blen = 0, alen = 0;
	/* Fetching processes, and get populated list and population */
	blen = fetch_processes(before);
	printf("BEFORE: %d processes\n", blen);

	/* We got a short list. */
	if (blen < 1) {
		free_list(before);
		free_list(after);

		fprintf(stderr, "No population fetched to before list.");
		return EXIT_FAILURE;
	}

	/* Prints the processes */
	/*print_list(before, blen); */

	/* Time for a nap */
	sleep(TIME);
	/* alternatively: sleep(atoi(getenv("SLEEP"))) */

	/* Fetching processes, and get populated list and population */
	alen = fetch_processes(after);
	printf("AFTER:  %d processes\n", alen);

	/* We got a short list. */
	if (alen < 1) {
		free_list(before);
		free_list(after);

		fprintf(stderr, "No population fetched to after list.\n");
		return EXIT_FAILURE;
	}

	/* Prints the processes */
	/*print_list(after, alen); */

	/* Print the difference (if any) between lists */
	print_diff(before, after);

	/* Freeing the list */
	free_list(before);
	free_list(after);

	return EXIT_SUCCESS;
}
