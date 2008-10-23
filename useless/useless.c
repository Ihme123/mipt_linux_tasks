
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "../lib/lib.h"


int main ()
{
	char str [MAX_STRING_LEN + 1];
	int delay;
	char *cmd;
	pid_t pid;
	char *cmd_args [20];
	size_t len;
	FILE *fconf;
	int i;
	int running_count;
	int status; // child return status

	fconf = fopen ("useless.conf", "r");
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

			for (i = 0; cmd_args [i] != NULL; i ++)
				printf ("%s%s",
					i == 0 ? "Starting program: " : " ",
					cmd_args [i]);

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

	return 0;
}

