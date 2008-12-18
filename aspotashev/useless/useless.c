
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "../lib/lib.h"

#define APP_NAME "useless"

int useless_running (const char *sem_filename, int *semid)
{
	key_t key;

	if ((key = ftok (sem_filename, 0)) == -1) {
		err ("can't get IPC key");
		return 1;
	}
	if ((*semid = semget (key, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1) {
		if (errno == EEXIST) {
			err ("an instance of useless is already running");
		} else {
			err ("can't allocate semaphore");
		}

		return 1;
	}

	return 0;
}

int unlock_useless (int semid)
{
	if (semctl (semid, 0, IPC_RMID, 0) != 0) {
		err ("can't delete semaphore");
		return -1;
	}

	return 0;
}

int main ()
{
	char str [MAX_STRING_LEN + 1];
	int delay;
	char *cmd;
	pid_t pid;
	char *cmd_args [MAX_PROGRAM_ARGS];
	size_t len;
	FILE *fconf;
	int i;
	int running_count;
	int status; // child return status
	const char *conf_filename = "useless.conf";
	int lock_semid;

	if (useless_running (conf_filename, &lock_semid))
		return 1;

	fconf = fopen (conf_filename, "r");
	if (fconf == NULL)
	{
		printf ("configuration file not found!\n");
		return 1;
	}

	running_count = 0;
	while (fgets (str, MAX_STRING_LEN, fconf) != NULL) {
		if (sscanf (str, "%d", &delay) != 1) {
			printf ("unexpected end?\n");
			break;
		}

		len = strlen (str);
		if (str [len - 1] == '\n')
			str [len - 1] = 0;

		cmd = str;
		skip_space (cmd); // spaces before <delay>
		skip_not_space (cmd); // <delay>

		parse_args (cmd, cmd_args);

		pid = fork ();
		if (pid == 0) {
			sleep (delay);

			for (i = 0; cmd_args [i] != NULL; i ++) {
				printf ("%s%s",
					i == 0 ? "Starting program: " : " ",
					cmd_args [i]);
			}
			printf ("\n");

			execvp (cmd_args [0], cmd_args);
			printf ("execvp failed! (error: %s)\n", strerror (errno));
		} else if (pid < 0) {
			printf ("fork failed!\n");
		} else {
			// ok, parent
			running_count ++;
		}
	}
	fclose (fconf);

	while (running_count > 0) {
		wait (&status);
		running_count --;
	}

	if (unlock_useless (lock_semid) < 0)
		return 1;

	return 0;
}

