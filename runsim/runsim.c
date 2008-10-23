
#include <unistd.h>

#include "../lib/lib.h"

#define APP_NAME "runsim"

int try_run (char *cmd)
{
	pid_t pid;
	char *cmd_args [MAX_PROGRAM_ARGS];
	// TODO: check the number of running processes

	parse_args (cmd, cmd_args);

	pid = fork ();
	if (pid < 0) {
		err ();
		return -1;
	} else if (pid == 0) { // child
		execvp (cmd_args [0], cmd_args);
		err ("exec failed");
		return -1;
	} else { // parent
		usleep (100000);
		return 0;
	}
}

void usage ()
{
	err ("Usage: runsim <N>");
}

int main (int argc, char *argv [])
{
	int N;
	char reverse_conv [MAX_STRING_LEN];
	char cmd [MAX_STRING_LEN + 1];
	size_t len;

	if (argc != 2) {
		err ("this program takes 1 argument");
		usage ();
		return 1;
	}

	N = atoi (argv [1]);
	sprintf (reverse_conv, "%d", N);
	if (strcmp (argv [1], reverse_conv)) {
		err ("input argument presentation error: (%s), (%s)",
			argv [1], reverse_conv);
		usage ();
		return 1;
	}

	while (1) {
		if (fgets (cmd, MAX_STRING_LEN, stdin) == NULL) {
			break;
		}

		len = strlen (cmd);
		if (cmd [len - 1] != '\n') {
			err ("read 'half a string'...exit...");
			break;
		} else {
			cmd [len - 1] = 0;
			try_run (cmd);
		}
	}

	return 0;
}

